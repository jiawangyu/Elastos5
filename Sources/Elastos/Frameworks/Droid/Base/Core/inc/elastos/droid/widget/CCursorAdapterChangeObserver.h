
#ifndef __ELASTOS_DROID_WIDGET_CCURSORADAPTER_CHANGEOBSERVER_H__
#define __ELASTOS_DROID_WIDGET_CCURSORADAPTER_CHANGEOBSERVER_H__

#include "_Elastos_Droid_Widget_CCursorAdapterChangeObserver.h"
#include "elastos/droid/widget/CursorAdapter.h"

namespace Elastos {
namespace Droid {
namespace Widget {

CarClass(CCursorAdapterChangeObserver)
    , public CursorAdapter::ChangeObserver
{
public:
    CAR_OBJECT_DECL()
};

}// namespace Elastos
}// namespace Droid
}// namespace Widget

#endif // __ELASTOS_DROID_WIDGET_CCURSORADAPTER_CHANGEOBSERVER_H__
