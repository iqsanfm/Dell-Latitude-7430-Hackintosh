//  SMCDellDDV — read-only VirtualSMC plugin: fan RPM (+ thermal sensors as ioreg) from Dell WMI "DDV"
//  ACPI method \_SB.AMWV.WMDV (GUID 8A42EA14-4F2A-FD45-6422-0087F7A7E608, _UID "ADDV").
//  WMDV(0, 0x20, buf) -> Package{len, Buffer[n*3+1]}: per fan {type, rpm lo, rpm hi}, 0xFF terminator.
//  WMDV(0, 0x22, buf) -> Package{len, Buffer[n*5+1]}: per sensor {type, now, min, max, unk}, 0xFF terminator.
//  Never writes fan control. Kill switch boot-arg: -ddvoff. Debug: -ddvdbg.
#pragma once
#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <VirtualSMCSDK/kern_vsmcapi.hpp>
#include <Headers/kern_util.hpp>

static constexpr size_t DDVMaxFans = 4;
static constexpr size_t DDVMaxTemps = 8;
extern _Atomic(uint16_t) ddvFanRpm[DDVMaxFans];
extern _Atomic(int8_t) ddvTemp[DDVMaxTemps];

class DDVFanAc : public VirtualSMCValue {
	size_t index;
public:
	DDVFanAc(size_t i) : index(i) {}
protected:
	SMC_RESULT readAccess() override;
};

class DDVTemp : public VirtualSMCValue {
	size_t index;
public:
	DDVTemp(size_t i) : index(i) {}
protected:
	SMC_RESULT readAccess() override;
};

class EXPORT SMCDellDDV : public IOService {
	OSDeclareDefaultStructors(SMCDellDDV)

	IONotifier *vsmcNotifier {nullptr};
	IOWorkLoop *workloop {nullptr};
	IOTimerEventSource *poller {nullptr};
	IOACPIPlatformDevice *ddv {nullptr};

	static constexpr uint32_t PollMSDefault {2000};
	uint32_t PollMS {PollMSDefault};   // boot-arg ddvms=<500..60000>
	uint32_t StartDelayMS {60000};      // v8: first poll 60 s after VirtualSMC hookup; boot-arg ddvdelay=<0..600> seconds
	bool fanOnly {false};              // boot-arg -ddvfanonly: poll fans only (skip WMDV 0x22)

	// Timing of each poll (v4): ring of {start ns, fans ns, temps ns}, published as ioreg ddv-ring.
	struct PollRec { uint64_t startNs; uint32_t fansNs; uint32_t tempsNs; };
	static constexpr size_t RingSize {64};
	PollRec ring[RingSize] {};
	uint32_t ringPos {0};
	uint32_t pollCount {0};
	uint64_t fansMaxNs {0}, tempsMaxNs {0}, fansSumNs {0}, tempsSumNs {0};
	void publishTiming();

	static constexpr uint32_t FuncFans {0x20};
	static constexpr uint32_t FuncTemps {0x22};

	size_t fanCount {0};
	size_t tempCount {0};
	uint8_t fanType[DDVMaxFans] {};
	uint8_t tempType[DDVMaxTemps] {};

	VirtualSMCAPI::Plugin vsmcPlugin {
		xStringify(PRODUCT_NAME),
		parseModuleVersion(xStringify(MODULE_VERSION)),
		VirtualSMCAPI::Version,
	};

	OSData *callDDV(uint32_t func);
	bool refreshFans(bool publish);
	bool refreshTemps(bool publish);
	void refresh();
	static bool vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier);

public:
	IOService *probe(IOService *provider, SInt32 *score) override;
	bool start(IOService *provider) override;
	void stop(IOService *provider) override;
};
