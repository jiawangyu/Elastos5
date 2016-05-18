
#ifndef  __ELASTOS_DROID_SYSTEMUI_STATUSBAR_CNOTIFICATIONOVERFLOWCONTAINER_H__
#define  __ELASTOS_DROID_SYSTEMUI_STATUSBAR_CNOTIFICATIONOVERFLOWCONTAINER_H__

#include "_Elastos_Droid_SystemUI_StatusBar_CNotificationOverflowContainer.h"
#include "elastos/droid/systemui/statusbar/ActivatableNotificationView.h"

namespace Elastos {
namespace Droid {
namespace SystemUI {
namespace StatusBar {

/**
 * Container view for overflowing notification icons on Keyguard.
 */
CarClass(CNotificationOverflowContainer)
    , public ActivatableNotificationView
    , public INotificationOverflowContainer
{
public:
    CAR_OBJECT_DECL()

    CAR_INTERFACE_DECL()

    CARAPI constructor(
        /* [in] */ IContext* context,
        /* [in] */ IAttributeSet* attrs);

    CARAPI GetIconsView(
        /* [out] */ INotificationOverflowIconsView** view);

protected:
    // @Override
    CARAPI OnFinishInflate();

private:
    AutoPtr<INotificationOverflowIconsView> mIconsView;
};

} // namespace StatusBar
} // namespace SystemUI
} // namespace Droid
} // namespace Elastos

#endif // __ELASTOS_DROID_SYSTEMUI_STATUSBAR_CNOTIFICATIONOVERFLOWCONTAINER_H__
