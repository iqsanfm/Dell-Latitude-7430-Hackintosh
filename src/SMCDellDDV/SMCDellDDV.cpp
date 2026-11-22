#include "SMCDellDDV.hpp"
#include <Headers/kern_version.hpp>

OSDefineMetaClassAndStructors(SMCDellDDV, IOService)

bool ADDPR(debugEnabled) = false;
uint32_t ADDPR(debugPrintDelay) = 0;

_Atomic(uint16_t) ddvFanRpm[DDVMaxFans];
_Atomic(int8_t) ddvTemp[DDVMaxTemps];

SMC_RESULT DDVFanAc::readAccess() {
	uint16_t rpm = atomic_load_explicit(&ddvFanRpm[index], memory_order_acquire);
	*reinterpret_cast<uint16_t *>(data) = VirtualSMCAPI::encodeIntFp(SmcKeyTypeFpe2, rpm);
	return SmcSuccess;
}

SMC_RESULT DDVTemp::readAccess() {
	int8_t t = atomic_load_explicit(&ddvTemp[index], memory_order_acquire);
	*reinterpret_cast<uint16_t *>(data) = VirtualSMCAPI::encodeIntSp(SmcKeyTypeSp78, t);
	return SmcSuccess;
}

static constexpr SMC_KEY KeyFNum = SMC_MAKE_IDENTIFIER('F','N','u','m');
static constexpr char KeyIdx[] = "0123456789";
static constexpr SMC_KEY keyF(size_t i, char a, char b) { return SMC_MAKE_IDENTIFIER('F', KeyIdx[i], a, b); }
static constexpr SMC_KEY keyTD(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'D', KeyIdx[i], 'P'); }

struct FanDesc {
	uint8_t type {0};      // FAN_PWM_TACH
	uint8_t zone {1};
	uint8_t location {12}; // LEFT_MID_REAR
	uint8_t rsvd {0};
	char name[12] {};
};

// Calls \_SB.AMWV.WMDV(0, func, Buffer(4){0}) and returns the payload buffer (retained) or nullptr.
OSData *SMCDellDDV::callDDV(uint32_t func) {
	uint8_t zero[4] {};
	OSObject *params[3] {
		OSNumber::withNumber(0ULL, 32),
		OSNumber::withNumber(func, 32),
		OSData::withBytes(zero, sizeof(zero)),
	};
	OSObject *result = nullptr;
	IOReturn ret = kIOReturnNoMemory;
	if (params[0] && params[1] && params[2])
		ret = ddv->evaluateObject("WMDV", &result, params, 3);
	for (auto p : params) OSSafeReleaseNULL(p);
	if (ret != kIOReturnSuccess || !result) {
		DBGLOG("ddv", "WMDV(0x%x) failed 0x%x", func, ret);
		OSSafeReleaseNULL(result);
		return nullptr;
	}
	OSData *out = nullptr;
	if (auto arr = OSDynamicCast(OSArray, result)) {
		if (arr->getCount() >= 2) out = OSDynamicCast(OSData, arr->getObject(1));
	}
	if (out) out->retain();
	result->release();
	return out;
}

bool SMCDellDDV::refreshFans(bool publish) {
	auto buf = callDDV(FuncFans);
	if (!buf) return false;
	auto b = static_cast<const uint8_t *>(buf->getBytesNoCopy());
	size_t len = buf->getLength();
	size_t n = 0;
	auto arr = publish ? OSArray::withCapacity(DDVMaxFans) : nullptr;
	for (size_t off = 0; off + 3 <= len && n < DDVMaxFans && b[off] != 0xFF; off += 3, n++) {
		uint16_t rpm = static_cast<uint16_t>(b[off + 1] | (b[off + 2] << 8));
		fanType[n] = b[off];
		atomic_store_explicit(&ddvFanRpm[n], rpm, memory_order_release);
		if (arr) {
			auto d = OSDictionary::withCapacity(2);
			auto t = OSNumber::withNumber(b[off], 8), r = OSNumber::withNumber(rpm, 16);
			if (d && t && r) { d->setObject("type", t); d->setObject("rpm", r); arr->setObject(d); }
			OSSafeReleaseNULL(t); OSSafeReleaseNULL(r); OSSafeReleaseNULL(d);
		}
	}
	if (!publish) fanCount = n;
	if (arr) { setProperty("ddv-fans", arr); arr->release(); }
	buf->release();
	return n > 0;
}

bool SMCDellDDV::refreshTemps(bool publish) {
	auto buf = callDDV(FuncTemps);
	if (!buf) return false;
	auto b = static_cast<const uint8_t *>(buf->getBytesNoCopy());
	size_t len = buf->getLength();
	size_t n = 0;
	auto arr = publish ? OSArray::withCapacity(DDVMaxTemps) : nullptr;
	for (size_t off = 0; off + 5 <= len && n < DDVMaxTemps && b[off] != 0xFF; off += 5, n++) {
		tempType[n] = b[off];
		atomic_store_explicit(&ddvTemp[n], static_cast<int8_t>(b[off + 1]), memory_order_release);
		if (arr) {
			auto d = OSDictionary::withCapacity(5);
			OSNumber *v[5] { OSNumber::withNumber(b[off], 8), OSNumber::withNumber(b[off+1], 8),
				OSNumber::withNumber(b[off+2], 8), OSNumber::withNumber(b[off+3], 8), OSNumber::withNumber(b[off+4], 8) };
			const char *k[5] {"type", "now", "min", "max", "unk"};
			if (d) for (int i = 0; i < 5; i++) if (v[i]) d->setObject(k[i], v[i]);
			if (d) arr->setObject(d);
			for (auto x : v) OSSafeReleaseNULL(x);
			OSSafeReleaseNULL(d);
		}
	}
	if (!publish) tempCount = n;
	if (arr) { setProperty("ddv-temps", arr); arr->release(); }
	buf->release();
	return n > 0;
}

static inline uint64_t nowNs() {
	uint64_t ns = 0;
	absolutetime_to_nanoseconds(mach_absolute_time(), &ns);
	return ns;
}

static inline uint32_t clampNs(uint64_t v) { return v > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(v); }

void SMCDellDDV::refresh() {
	uint64_t t0 = nowNs();
	refreshFans(true);
	uint64_t t1 = nowNs();
	if (!fanOnly) refreshTemps(true);
	uint64_t t2 = nowNs();
	auto &r = ring[ringPos % RingSize];
	r.startNs = t0;
	r.fansNs = clampNs(t1 - t0);
	r.tempsNs = clampNs(t2 - t1);
	ringPos++; pollCount++;
	fansSumNs += r.fansNs; tempsSumNs += r.tempsNs;
	if (r.fansNs > fansMaxNs) { fansMaxNs = r.fansNs; if (fansMaxNs > 20000000) SYSLOG("ddv", "timing: new max fans %llu us", fansMaxNs / 1000); }
	if (r.tempsNs > tempsMaxNs) { tempsMaxNs = r.tempsNs; if (tempsMaxNs > 20000000) SYSLOG("ddv", "timing: new max temps %llu us", tempsMaxNs / 1000); }
	// v7: publishTiming() dimatikan (A/B bug Wi-Fi boot: v4+ gagal FW_LOADED); hitungan timing tetap di memori
	if (poller) poller->setTimeoutMS(PollMS);
}

void SMCDellDDV::publishTiming() {
	// ddv-ring: RingSize records in chronological order (oldest first), each {u64 startNs, u32 fansNs, u32 tempsNs}
	PollRec ordered[RingSize];
	for (size_t i = 0; i < RingSize; i++) ordered[i] = ring[(ringPos + i) % RingSize];
	if (auto d = OSData::withBytes(ordered, sizeof(ordered))) { setProperty("ddv-ring", d); d->release(); }
	auto dict = OSDictionary::withCapacity(8);
	if (!dict) return;
	uint64_t v[] { pollCount, PollMS, fanOnly,
		ring[(ringPos + RingSize - 1) % RingSize].fansNs / 1000ULL, fansMaxNs / 1000, pollCount ? fansSumNs / pollCount / 1000 : 0,
		ring[(ringPos + RingSize - 1) % RingSize].tempsNs / 1000ULL, tempsMaxNs / 1000, pollCount ? tempsSumNs / pollCount / 1000 : 0 };
	const char *k[] { "polls", "interval-ms", "fan-only", "fans-last-us", "fans-max-us", "fans-avg-us", "temps-last-us", "temps-max-us", "temps-avg-us" };
	for (size_t i = 0; i < arrsize(v); i++)
		if (auto n = OSNumber::withNumber(v[i], 64)) { dict->setObject(k[i], n); n->release(); }
	setProperty("ddv-timing", dict);
	dict->release();
}

IOService *SMCDellDDV::probe(IOService *provider, SInt32 *score) {
	if (!IOService::probe(provider, score)) return nullptr;

	auto dict = IOService::nameMatching("AppleACPIPlatformExpert");
	if (!dict) return nullptr;
	auto acpi = IOService::waitForMatchingService(dict);
	dict->release();
	if (!acpi) return nullptr;
	acpi->release();

	// EC OperationRegion handler must be up before WMDV touches the EC
	dict = IOService::serviceMatching("AppleACPIEC");
	if (!dict) return nullptr;
	auto ec = IOService::waitForMatchingService(dict, 15ULL * 1000 * 1000 * 1000);
	dict->release();
	if (!ec) { SYSLOG("ddv", "AppleACPIEC not found"); return nullptr; }
	ec->release();

	dict = IOService::nameMatching("PNP0C14");
	if (!dict) return nullptr;
	auto it = IOService::getMatchingServices(dict);
	dict->release();
	if (!it) return nullptr;
	while (auto obj = it->getNextObject()) {
		auto dev = OSDynamicCast(IOACPIPlatformDevice, obj);
		if (!dev) continue;
		auto uid = OSDynamicCast(OSString, dev->getProperty("_UID"));
		if (uid && uid->isEqualTo("ADDV")) { ddv = dev; ddv->retain(); break; }
	}
	it->release();
	if (!ddv) { SYSLOG("ddv", "no PNP0C14 with _UID ADDV"); return nullptr; }
	if (ddv->validateObject("WMDV") != kIOReturnSuccess) { SYSLOG("ddv", "no WMDV method"); return nullptr; }

	uint32_t ms = 0;
	if (PE_parse_boot_argn("ddvms", &ms, sizeof(ms)) && ms >= 500 && ms <= 60000) PollMS = ms;
	uint32_t dly = 0;
	if (PE_parse_boot_argn("ddvdelay", &dly, sizeof(dly)) && dly <= 600) StartDelayMS = dly * 1000;
	fanOnly = checkKernelArgument("-ddvfanonly");
	SYSLOG("ddv", "poll interval %u ms, start delay %u ms, fan-only %d", PollMS, StartDelayMS, fanOnly);

	bool fansOk = refreshFans(false);
	refreshTemps(false);
	SYSLOG("ddv", "fans %lu (ok %d) temps %lu", fanCount, fansOk, tempCount);
	if (!fansOk && tempCount == 0) return nullptr;

	VirtualSMCAPI::addKey(KeyFNum, vsmcPlugin.data, VirtualSMCAPI::valueWithUint8(static_cast<uint8_t>(fanCount), nullptr, SMC_KEY_ATTRIBUTE_CONST | SMC_KEY_ATTRIBUTE_READ));
	for (size_t i = 0; i < fanCount; i++) {
		FanDesc desc;
		snprintf(desc.name, sizeof(desc.name), fanType[i] == 0 ? "CPU Fan" : "Fan %lu", i + 1);
		VirtualSMCAPI::addKey(keyF(i, 'I', 'D'), vsmcPlugin.data, VirtualSMCAPI::valueWithData(
			reinterpret_cast<const SMC_DATA *>(&desc), sizeof(desc), SmcKeyTypeFds, nullptr, SMC_KEY_ATTRIBUTE_CONST | SMC_KEY_ATTRIBUTE_READ));
		VirtualSMCAPI::addKey(keyF(i, 'A', 'c'), vsmcPlugin.data, VirtualSMCAPI::valueWithFp(0, SmcKeyTypeFpe2, new DDVFanAc(i), SMC_KEY_ATTRIBUTE_READ));
		VirtualSMCAPI::addKey(keyF(i, 'M', 'n'), vsmcPlugin.data, VirtualSMCAPI::valueWithFp(0, SmcKeyTypeFpe2, nullptr, SMC_KEY_ATTRIBUTE_CONST | SMC_KEY_ATTRIBUTE_READ));
		uint32_t maxRpm = 6000;
		if (auto m = OSDynamicCast(OSNumber, getProperty("FanMaxRPM"))) maxRpm = m->unsigned32BitValue();
		VirtualSMCAPI::addKey(keyF(i, 'M', 'x'), vsmcPlugin.data, VirtualSMCAPI::valueWithFp(maxRpm, SmcKeyTypeFpe2, nullptr, SMC_KEY_ATTRIBUTE_CONST | SMC_KEY_ATTRIBUTE_READ));
		VirtualSMCAPI::addKey(keyF(i, 'M', 'd'), vsmcPlugin.data, VirtualSMCAPI::valueWithUint8(0, nullptr, SMC_KEY_ATTRIBUTE_CONST | SMC_KEY_ATTRIBUTE_READ));
	}
	// Thermal sensors, keyed by DDV sensor type (dell-wmi-ddv labels) onto keys Stats/iStat know:
	//   0x22 Memory -> TMA1.., 0x44 Ambient -> TA0P.., 0x33 Other -> TZ0C.. ("Thermal zone"),
	//   anything else (0x00 CPU: TC0P/TC0H already taken by SMCProcessor) -> TD<i>P.
	size_t nMem = 0, nAmb = 0, nOth = 0;
	for (size_t i = 0; i < tempCount; i++) {
		SMC_KEY k;
		switch (tempType[i]) {
			case 0x22: k = nMem < 4 ? SMC_MAKE_IDENTIFIER('T', 'M', 'A', KeyIdx[1 + nMem++]) : keyTD(i); break;
			case 0x44: k = SMC_MAKE_IDENTIFIER('T', 'A', KeyIdx[nAmb++], 'P'); break;
			case 0x33: k = SMC_MAKE_IDENTIFIER('T', 'Z', KeyIdx[nOth++], 'C'); break;
			default:   k = keyTD(i); break;
		}
		VirtualSMCAPI::addKey(k, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new DDVTemp(i), SMC_KEY_ATTRIBUTE_READ));
	}
	// VirtualSMC looks keys up by binary search: plugin keys MUST be sorted (same as SMCProcessor),
	// otherwise e.g. F0Ac/F0Md (added after F0ID) are enumerable but unreadable.
	qsort(const_cast<VirtualSMCKeyValue *>(vsmcPlugin.data.data()), vsmcPlugin.data.size(), sizeof(VirtualSMCKeyValue), VirtualSMCKeyValue::compare);
	return this;
}

bool SMCDellDDV::start(IOService *provider) {
	if (!IOService::start(provider)) return false;
	setProperty("VersionInfo", kextVersion);
	setProperty("ddv-fan-count", fanCount, 8);
	setProperty("ddv-temp-count", tempCount, 8);
	vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
	return vsmcNotifier != nullptr;
}

bool SMCDellDDV::vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier) {
	if (!sensors || !vsmc) return false;
	auto self = static_cast<SMCDellDDV *>(sensors);
	auto ret = vsmc->callPlatformFunction(VirtualSMCAPI::SubmitPlugin, true, sensors, &self->vsmcPlugin, nullptr, nullptr);
	if (ret != kIOReturnSuccess) {
		if (ret != kIOReturnUnsupported) SYSLOG("ddv", "plugin submission failure %X", ret);
		return false;
	}
	// v5: dedicated workloop. getWorkLoop() here resolves (via IOResources) to the platform expert's
	// shared workloop; a 85-200 ms WMDV evaluation on it stalls the I2C trackpad stack (RMII2C/VoodooI2C)
	// -> lift+touch reports merge -> cursor jump. Own thread keeps the slow ACPI call off that gate.
	self->workloop = IOWorkLoop::workLoop();
	self->setProperty("ddv-workloop", self->workloop ? "own" : "none");
	self->poller = IOTimerEventSource::timerEventSource(self, [](OSObject *object, IOTimerEventSource *) {
		if (auto s = OSDynamicCast(SMCDellDDV, object)) s->refresh();
	});
	if (!self->poller || !self->workloop || self->workloop->addEventSource(self->poller) != kIOReturnSuccess) {
		SYSLOG("ddv", "failed to set up poller");
		return false;
	}
	// v8: keep WMDV (ACPI/EC) polling out of the busy boot window (BT fw upload, Wi-Fi init, I2C
	// trackpad bring-up) -- suspected early-boot hard hang on v7 (2026-11-21 ~19:18).
	self->setProperty("ddv-start-delay-ms", self->StartDelayMS, 32);
	self->poller->setTimeoutMS(self->StartDelayMS ? self->StartDelayMS : self->PollMS);
	return true;
}

void SMCDellDDV::stop(IOService *provider) {
	PANIC("ddv", "called stop!!!");
}

EXPORT extern "C" kern_return_t ADDPR(kern_start)(kmod_info_t *, void *) {
	if (checkKernelArgument("-ddvoff")) return KERN_FAILURE;
	ADDPR(debugEnabled) = checkKernelArgument("-ddvdbg") || checkKernelArgument("-liludbgall");
	return KERN_SUCCESS;
}

EXPORT extern "C" kern_return_t ADDPR(kern_stop)(kmod_info_t *, void *) {
	return KERN_FAILURE;
}
