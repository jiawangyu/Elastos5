#ifndef __ELASTOS_APPS_DIALER_WIDGET_ACTIONBARCONTROLLER_H__
#define __ELASTOS_APPS_DIALER_WIDGET_ACTIONBARCONTROLLER_H__

#include "_Elastos.Apps.Dialer.h"
#include <elastos/core/Object.h>
#include "Elastos.Droid.Animation.h"

using Elastos::Droid::Animation::IValueAnimator;

namespace Elastos {
namespace Apps {
namespace Dialer {
namespace Widget {

/**
 * Controls the various animated properties of the actionBar: showing/hiding, fading/revealing,
 * and collapsing/expanding, and assigns suitable properties to the actionBar based on the
 * current state of the UI.
 */
class ActionBarController
    : public Object
    , public IActionBarController
{
private:
    class MyAnimatorUpdateListener
        : public Object
        , public IAnimatorUpdateListener
    {
    public:
        CAR_INTERFACE_DECL();

        MyAnimatorUpdateListener(
            /* [in] */ ActionBarController* host);

        // @Override
        CARAPI OnAnimationUpdate(
            /* [in] */ IValueAnimator* animation);

    private:
        ActionBarController* mHost;
    };

public:
    CAR_INTERFACE_DECL()

    CARAPI constructor(
        /* [in] */ IActionBarControllerActivityUi* activityUi,
        /* [in] */ ISearchEditTextLayout* searchBox);

    /**
     * @return Whether or not the action bar is currently showing (both slid down and visible)
     */
    CARAPI IsActionBarShowing(
        /* [out] */ Boolean* result);

    /**
     * Called when the user has tapped on the collapsed search box, to start a new search query.
     */
    CARAPI OnSearchBoxTapped();

    /**
     * Called when search UI has been exited for some reason.
     */
    CARAPI OnSearchUiExited();

    /**
     * Called to indicate that the user is trying to hide the dialpad. Should be called before
     * any state changes have actually occurred.
     */
    CARAPI OnDialpadDown();

    /**
     * Called to indicate that the user is trying to show the dialpad. Should be called before
     * any state changes have actually occurred.
     */
    CARAPI OnDialpadUp();

    CARAPI SlideActionBar(
        /* [in] */ Boolean slideUp,
        /* [in] */ Boolean animate);

    CARAPI SetAlpha(
        /* [in] */ Float alphaValue);

    CARAPI SetHideOffset(
        /* [in] */ Int32 offset);

    /**
     * @return The offset the action bar is being translated upwards by
     */
    CARAPI GetHideOffset(
        /* [out] */ Int32* offset);

    /**
     * Saves the current state of the action bar into a provided {@link Bundle}
     */
    CARAPI SaveInstanceState(
        /* [in] */ IBundle* outState);

    /**
     * Restores the action bar state from a provided {@link Bundle}.
     */
    CARAPI RestoreInstanceState(
        /* [in] */ IBundle* inState);

    /**
     * This should be called after onCreateOptionsMenu has been called, when the actionbar has
     * been laid out and actually has a height.
     */
    CARAPI RestoreActionBarOffset();

    // @VisibleForTesting
    CARAPI GetIsActionBarSlidUp(
        /* [out] */ Boolean* result);

public:
    static const Boolean DEBUG; // = DialtactsActivity.DEBUG;
    static const String TAG; // = "ActionBarController";

private:
    static const String KEY_IS_SLID_UP; // = "key_actionbar_is_slid_up";
    static const String KEY_IS_FADED_OUT; // = "key_actionbar_is_faded_out";
    static const String KEY_IS_EXPANDED; // = "key_actionbar_is_expanded";

    AutoPtr<IActionBarControllerActivityUi> mActivityUi;
    AutoPtr<ISearchEditTextLayout> mSearchBox;

    Boolean mIsActionBarSlidUp;

    // AutoPtr<IAnimationCallback> mFadeOutCallback;
};

} // Widget
} // Dialer
} // Apps
} // Elastos

#endif //__ELASTOS_APPS_DIALER_WIDGET_ACTIONBARCONTROLLER_H__
