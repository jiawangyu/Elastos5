
#ifndef __ELASTOS_DROID_VIEW_CVIEWBASESAVEDSTATE_H__
#define __ELASTOS_DROID_VIEW_CVIEWBASESAVEDSTATE_H__

#include "_Elastos_Droid_View_CViewBaseSavedState.h"
#include "elastos/droid/view/View.h"

using Elastos::Droid::View::View;

namespace Elastos {
namespace Droid {
namespace View {

CarClass(CViewBaseSavedState)
    , public View::BaseSavedState
{
public:
    CAR_OBJECT_DECL()
};

}// namespace View
}// namespace Droid
}// namespace Elastos

#endif //__ELASTOS_DROID_VIEW_CVIEWBASESAVEDSTATE_H__