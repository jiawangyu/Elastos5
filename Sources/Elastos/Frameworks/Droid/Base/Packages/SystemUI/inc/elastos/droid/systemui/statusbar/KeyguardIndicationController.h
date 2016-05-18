
#ifndef  __ELASTOS_DROID_SYSTEMUI_STATUSBAR_KEYGUARDINDICATIONCONTROLLER_H__
#define  __ELASTOS_DROID_SYSTEMUI_STATUSBAR_KEYGUARDINDICATIONCONTROLLER_H__

#include "_Elastos.Droid.SystemUI.h"
#include <elastos/droid/content/BroadcastReceiver.h>
#include <elastos/droid/os/Handler.h>
#include <elastos/core/Object.h>

using Elastos::Droid::Content::BroadcastReceiver;
using Elastos::Droid::Content::IContext;
using Elastos::Droid::Content::IIntent;
using Elastos::Droid::Internal::App::IIBatteryStats;
using Elastos::Droid::Os::Handler;
using Elastos::Droid::SystemUI::StatusBar::Phone::IKeyguardIndicationTextView;
using Elastos::Core::Object;

namespace Elastos {
namespace Droid {
namespace SystemUI {
namespace StatusBar {

/**
 * Controls the little text indicator on the keyguard.
 */
class KeyguardIndicationController
    : public Object
    , public IKeyguardIndicationController
{
private:
    class ControllerBroadcastReceiver: public BroadcastReceiver
    {
    public:
        ControllerBroadcastReceiver(
            /* [in] */ KeyguardIndicationController* host);

        // @Override
        CARAPI OnReceive(
            /* [in] */ IContext* context,
            /* [in] */ IIntent* intent);

        CARAPI ToString(
            /* [out] */ String* str)
        {
            *str = String("KeyguardIndicationController.ControllerBroadcastReceiver");
            return NOERROR;
        }

    private:
        KeyguardIndicationController* mHost;
    };

    class ControllerHandler: public Handler
    {
    public:
        ControllerHandler(
            /* [in] */ KeyguardIndicationController* host);

        // @Override
        CARAPI HandleMessage(
            /* [in] */ IMessage* msg);

    private:
        KeyguardIndicationController* mHost;
    };

public:
    CAR_INTERFACE_DECL()

    KeyguardIndicationController(
        /* [in] */ IContext* context,
        /* [in] */ IKeyguardIndicationTextView* textView);

    CARAPI SetVisible(
        /* [in] */ Boolean visible);

    /**
     * Sets the indication that is shown if nothing else is showing.
     */
    CARAPI SetRestingIndication(
        /* [in] */ const String& restingIndication);

    /**
     * Hides transient indication in {@param delayMs}.
     */
    CARAPI HideTransientIndicationDelayed(
        /* [in] */ Int64 delayMs);

    /**
     * Shows {@param transientIndication} until it is hidden by {@link #hideTransientIndication}.
     */
    CARAPI ShowTransientIndication(
        /* [in] */ Int32 transientIndication);

    /**
     * Shows {@param transientIndication} until it is hidden by {@link #hideTransientIndication}.
     */
    CARAPI ShowTransientIndication(
        /* [in] */ const String& transientIndication);

    /**
     * Hides transient indication.
     */
    CARAPI HideTransientIndication();

private:
    CARAPI_(void) UpdateIndication();

    CARAPI_(String) ComputeIndication();

    CARAPI_(String) ComputePowerIndication();

private:
    static const String TAG;
    static const Int32 MSG_HIDE_TRANSIENT;

    AutoPtr<IContext> mContext;
    AutoPtr<IKeyguardIndicationTextView> mTextView;
    AutoPtr<IIBatteryStats> mBatteryInfo;

    String mRestingIndication;
    String mTransientIndication;
    Boolean mVisible;

    Boolean mPowerPluggedIn;
    Boolean mPowerCharged;

    // KeyguardUpdateMonitorCallback mUpdateMonitor = new KeyguardUpdateMonitorCallback() {
    //     @Override
    //     public void onRefreshBatteryInfo(KeyguardUpdateMonitor.BatteryStatus status) {
    //         mPowerPluggedIn = status.status == BatteryManager.BATTERY_STATUS_CHARGING
    //                 || status.status == BatteryManager.BATTERY_STATUS_FULL;
    //         mPowerCharged = status.isCharged();
    //         updateIndication();
    //     }
    // };

    AutoPtr<IBroadcastReceiver> mReceiver;

    AutoPtr<IHandler> mHandler;
};

} // namespace StatusBar
} // namespace SystemUI
} // namespace Droid
} // namespace Elastos

#endif // __ELASTOS_DROID_SYSTEMUI_STATUSBAR_KEYGUARDINDICATIONCONTROLLER_H__
