
#include "Elastos.Droid.Content.h"
#include "elastos/droid/transition/CFade.h"
#include "elastos/droid/transition/CTransitionValues.h"
#include "elastos/droid/animation/ObjectAnimator.h"
#include "elastos/droid/R.h"

#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::R;
using Elastos::Droid::Animation::IObjectAnimator;
using Elastos::Droid::Animation::ObjectAnimator;
using Elastos::Droid::Animation::IAnimatorPauseListener;
using Elastos::Droid::Content::Res::ITypedArray;
using Elastos::Droid::Transition::CTransitionValues;

using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace Transition {

Boolean CFade::DBG = Transition::DBG && FALSE;

const String CFade::TAG("Fade");

Int32 CFade::IN = Visibility::MODE_IN;

Int32 CFade::OUT = Visibility::MODE_OUT;

//===============================================================
// CFade::
//===============================================================
CAR_OBJECT_IMPL(CFade)

CAR_INTERFACE_IMPL(CFade, Visibility, IFade)

ECode CFade::constructor()
{
    return NOERROR;
}

ECode CFade::constructor(
    /* [in] */ Int32 fadingMode)
{
    SetMode(fadingMode);
    return NOERROR;
}

ECode CFade::constructor(
    /* [in] */ IContext* context,
    /* [in] */ IAttributeSet* attrs)
{
    Visibility::constructor(context, attrs);
    AutoPtr<ArrayOf<Int32> > attrIds = ArrayOf<Int32>::Alloc(
        const_cast<Int32*>(R::styleable::Fade),
        ArraySize(R::styleable::Fade));
    AutoPtr<ITypedArray> a;
    context->ObtainStyledAttributes(attrs, attrIds, (ITypedArray**)&a);
    Int32 m = 0;
    GetMode(&m);
    Int32 fadingMode = 0;
    a->GetInt32(R::styleable::Fade_fadingMode, m, &fadingMode);
    SetMode(fadingMode);
    return NOERROR;
}

AutoPtr<IAnimator> CFade::CreateAnimation(
    /* [in] */ IView* view,
    /* [in] */ Float startAlpha,
    /* [in] */ Float endAlpha)
{
    if (startAlpha == endAlpha) {
        return NULL;
    }
    view->SetTransitionAlpha(startAlpha);
    AutoPtr<ArrayOf<Float> > arr = ArrayOf<Float>::Alloc(1);
    (*arr)[0] = endAlpha;
    AutoPtr<IObjectAnimator> anim = ObjectAnimator::OfFloat(view, String("transitionAlpha"), arr);
    if (DBG) {
        Logger::D(TAG, "Created animator %p", anim.Get());
    }
    AutoPtr<FadeAnimatorListener> listener = new FadeAnimatorListener(view);
    IAnimator::Probe(anim)->AddListener(listener);
    IAnimator::Probe(anim)->AddPauseListener(IAnimatorPauseListener::Probe(listener));
    return IAnimator::Probe(anim);
}

ECode CFade::OnAppear(
    /* [in] */ IViewGroup* sceneRoot,
    /* [in] */ IView* view,
    /* [in] */ ITransitionValues* startValues,
    /* [in] */ ITransitionValues* endValues,
    /* [out] */ IAnimator** result)
{
    VALIDATE_NOT_NULL(result)
    if (DBG) {
        AutoPtr<IView> startView = (startValues != NULL) ? ((CTransitionValues*)startValues)->mView : NULL;
        Logger::D(LOG_TAG, "Fade.onAppear: startView, startVis, endView, endVis = %p, %p",
                startView.Get(), view);
    }
    AutoPtr<IAnimator> p = CreateAnimation(view, 0, 1);
    *result = p;
    REFCOUNT_ADD(*result)
    return NOERROR;
}

ECode CFade::OnDisappear(
    /* [in] */ IViewGroup* sceneRoot,
    /* [in] */ IView* view,
    /* [in] */ ITransitionValues* startValues,
    /* [in] */ ITransitionValues* endValues,
    /* [out] */ IAnimator** result)
{
    VALIDATE_NOT_NULL(result)
    AutoPtr<IAnimator> p = CreateAnimation(view, 1, 0);
    *result = p;
    REFCOUNT_ADD(*result)
    return NOERROR;
}

ECode CFade::Clone(
    /* [out] */ IInterface** obj)
{
    VALIDATE_NOT_NULL(obj)

    AutoPtr<IFade> changes;
    CFade::New((IFade**)&changes);

    CloneImpl(changes);
    *obj = changes;
    REFCOUNT_ADD(*obj)
    return NOERROR;
}

ECode CFade::CloneImpl(
    /* [in] */ IFade* obj)
{
    VALIDATE_NOT_NULL(obj);

    return Transition::CloneImpl(ITransition::Probe(obj));
}

//===============================================================
// CFade::FadeAnimatorListener::
//===============================================================

CFade::FadeAnimatorListener::FadeAnimatorListener(
    /* [in] */ IView* view)
{
    mView = view;
}

ECode CFade::FadeAnimatorListener::OnAnimationStart(
    /* [in] */ IAnimator* animator)
{
    Boolean bOverlap = FALSE;
    Int32 type = 0;
    mView->GetLayerType(&type);
    if ((mView->HasOverlappingRendering(&bOverlap), bOverlap) &&
        type == IView::LAYER_TYPE_NONE) {
        mLayerTypeChanged = TRUE;
        mView->SetLayerType(IView::LAYER_TYPE_HARDWARE, NULL);
    }
    return NOERROR;
}

ECode CFade::FadeAnimatorListener::OnAnimationCancel(
    /* [in] */ IAnimator* animator)
{
    mCanceled = TRUE;
    if (mPausedAlpha >= 0) {
        mView->SetTransitionAlpha(mPausedAlpha);
    }
    return NOERROR;
}

ECode CFade::FadeAnimatorListener::OnAnimationEnd(
    /* [in] */ IAnimator* animator)
{
    if (!mCanceled) {
        mView->SetTransitionAlpha(1);
    }
    if (mLayerTypeChanged) {
        mView->SetLayerType(IView::LAYER_TYPE_NONE, NULL);
    }
    return NOERROR;
}

ECode CFade::FadeAnimatorListener::OnAnimationPause(
    /* [in] */ IAnimator* animator)
{
    mView->GetTransitionAlpha(&mPausedAlpha);
    mView->SetTransitionAlpha(1);
    return NOERROR;
}

ECode CFade::FadeAnimatorListener::OnAnimationResume(
    /* [in] */ IAnimator* animator)
{
    mView->SetTransitionAlpha(mPausedAlpha);
    return NOERROR;
}

} // namespace Transition
} // namepsace Droid
} // namespace Elastos