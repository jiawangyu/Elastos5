
#include "elastos/droid/server/wm/CWindowManagerService.h"
#include "elastos/droid/server/wm/StartingData.h"
#include "elastos/droid/server/wm/ScreenRotationAnimation.h"
#include "elastos/droid/server/wm/KeyguardDisableHandler.h"
#include "elastos/droid/server/wm/ViewServer.h"
#include "elastos/droid/server/wm/DragState.h"
#include "elastos/droid/server/wm/InputMonitor.h"
#include "elastos/droid/server/wm/FakeWindowImpl.h"
#include "elastos/droid/server/wm/DisplayContent.h"
#include "elastos/droid/server/DisplayThread.h"
#include "elastos/droid/server/UiThread.h"
#include "elastos/droid/server/LocalServices.h"
#include "elastos/droid/os/Process.h"
#include "elastos/droid/os/SystemClock.h"
#include "elastos/droid/os/Binder.h"
#include "elastos/droid/os/Looper.h"
#include "elastos/droid/os/Build.h"
#include "elastos/droid/provider/Settings.h"
#include "elastos/droid/app/ActivityManagerNative.h"
#include "elastos/droid/server/am/CActivityManagerService.h"
#include "elastos/droid/server/am/CBatteryStatsService.h"
#include "elastos/droid/server/power/ShutdownThread.h"
#include "elastos/droid/utility/TimeUtils.h"
#include <Elastos.Droid.Provider.h>
#include <Elastos.CoreLibrary.Text.h>
#include <Elastos.CoreLibrary.Net.h>
#include "elastos/droid/R.h"
#include "elastos/droid/Manifest.h"
#include <elastos/core/Thread.h>
#include <elastos/core/Math.h>
#include <elastos/utility/logging/Slogger.h>
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#include <elastos/core/StringUtils.h>

using Elastos::Droid::Animation::IValueAnimatorHelper;
using Elastos::Droid::Animation::CValueAnimatorHelper;
using Elastos::Droid::App::IActivityOptions;
using Elastos::Droid::App::ActivityManagerNative;
using Elastos::Droid::App::IStatusBarManager;
using Elastos::Droid::App::EIID_IAppOpsManagerOnOpChangedInternalListener;
using Elastos::Droid::App::EIID_IAppOpsManagerOnOpChangedListener;
using Elastos::Droid::App::Admin::IDevicePolicyManager;
using Elastos::Droid::Content::IIntent;
using Elastos::Droid::Content::IIntentFilter;
using Elastos::Droid::Content::CIntentFilter;
using Elastos::Droid::Content::IContentResolver;
using Elastos::Droid::Content::Res::CConfiguration;
using Elastos::Droid::Content::Res::IConfigurationHelper;
using Elastos::Droid::Content::Res::CConfigurationHelper;
using Elastos::Droid::Content::Res::ICompatibilityInfoHelper;
using Elastos::Droid::Content::Res::CCompatibilityInfoHelper;
using Elastos::Droid::Content::Pm::IActivityInfo;
using Elastos::Droid::Content::Pm::IPackageManager;
using Elastos::Droid::Database::EIID_IContentObserver;
using Elastos::Droid::Graphics::CPoint;
using Elastos::Droid::Graphics::CRect;
using Elastos::Droid::Graphics::CRegion;
using Elastos::Droid::Graphics::CRectF;
using Elastos::Droid::Graphics::IPixelFormat;
using Elastos::Droid::Graphics::BitmapConfig;
using Elastos::Droid::Graphics::IBitmapFactory;
using Elastos::Droid::Graphics::CBitmapFactory;
using Elastos::Droid::Graphics::CMatrix;
using Elastos::Droid::Graphics::ICanvas;
using Elastos::Droid::Graphics::CCanvas;
using Elastos::Droid::Graphics::BitmapConfig_ALPHA_8;
using Elastos::Droid::Hardware::Display::EIID_IDisplayListener;
using Elastos::Droid::Hardware::Display::EIID_IDisplayManagerInternal;
using Elastos::Droid::Internal::Os::ISomeArgs;
using Elastos::Droid::Internal::Os::ISomeArgsHelper;
using Elastos::Droid::Internal::Os::CSomeArgsHelper;
using Elastos::Droid::Internal::Policy::IPolicyManager;
using Elastos::Droid::Internal::Policy::CPolicyManager;
using Elastos::Droid::Internal::Utility::IFastPrintWriter;
using Elastos::Droid::Internal::Utility::CFastPrintWriter;
using Elastos::Droid::Os::Looper;
using Elastos::Droid::Os::SystemClock;
using Elastos::Droid::Os::Binder;
using Elastos::Droid::Os::Process;
using Elastos::Droid::Os::EIID_ILowPowerModeListener;
using Elastos::Droid::Os::ISystemProperties;
using Elastos::Droid::Os::CSystemProperties;
using Elastos::Droid::Os::CBinder;
using Elastos::Droid::Os::ITrace;
using Elastos::Droid::Os::IMessage;
using Elastos::Droid::Os::CHandler;
using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Droid::Os::EIID_IPowerManagerInternal;
using Elastos::Droid::Os::IProcess;
using Elastos::Droid::Os::Build;
using Elastos::Droid::Os::CWorkSource;
using Elastos::Droid::Os::IWorkSource;
using Elastos::Droid::Os::ISystemService;
using Elastos::Droid::Os::CSystemService;
using Elastos::Droid::Os::IWorkSource;
// using Elastos::Droid::Os::CTrace;
using Elastos::Droid::Provider::ISettingsGlobal;
using Elastos::Droid::Provider::ISettingsSecure;
using Elastos::Droid::Provider::Settings;
using Elastos::Droid::Server::DisplayThread;
using Elastos::Droid::Server::UiThread;
using Elastos::Droid::Server::LocalServices;
using Elastos::Droid::Server::Am::CActivityManagerService;
using Elastos::Droid::Server::Am::CBatteryStatsService;
using Elastos::Droid::Server::Power::ShutdownThread;
using Elastos::Droid::Utility::ITypedValueHelper;
using Elastos::Droid::Utility::CTypedValueHelper;
using Elastos::Droid::Utility::ITypedValue;
using Elastos::Droid::Utility::CDisplayMetrics;
using Elastos::Droid::Utility::ISparseArray;
using Elastos::Droid::Utility::CSparseArray;
using Elastos::Droid::Utility::CArraySet;
using Elastos::Droid::Utility::ISparseInt32Array;
using Elastos::Droid::Utility::CSparseInt32Array;
using Elastos::Droid::Utility::TimeUtils;
using Elastos::Droid::View::IWindowInfo;
using Elastos::Droid::View::CWindowInfo;
using Elastos::Droid::View::IWindowInfoHelper;
using Elastos::Droid::View::CWindowInfoHelper;
using Elastos::Droid::View::IMotionEvent;
using Elastos::Droid::View::IWindowManagerPolicyWindowManagerFuncs;
using Elastos::Droid::View::IChoreographerHelper;
using Elastos::Droid::View::CChoreographerHelper;
using Elastos::Droid::View::IInputChannelHelper;
using Elastos::Droid::View::CInputChannelHelper;
using Elastos::Droid::View::IWindowManagerGlobal;
using Elastos::Droid::View::IInternalInsetsInfo;
using Elastos::Droid::View::CInputChannel;
using Elastos::Droid::View::IWindowInfoHelper;
using Elastos::Droid::View::IInputDevice;
using Elastos::Droid::View::ISurfaceControlHelper;
using Elastos::Droid::View::CSurfaceControlHelper;
using Elastos::Droid::View::CSurfaceSession;
using Elastos::Droid::View::IGravity;
using Elastos::Droid::View::CSurface;
using Elastos::Droid::View::IMagnificationSpecHelper;
using Elastos::Droid::View::CMagnificationSpecHelper;
using Elastos::Droid::View::CWindowContentFrameStats;
using Elastos::Droid::View::EIID_IOnKeyguardExitResult;
using Elastos::Droid::View::EIID_IIWindowManager;
using Elastos::Droid::View::EIID_IWindowManagerPolicyWindowManagerFuncs;
using Elastos::Droid::View::EIID_IWindowManagerInternal;
using Elastos::Droid::View::EIID_IWindowManagerPolicy;
using Elastos::Droid::View::IViewGroupLayoutParams;
using Elastos::Droid::View::CSurfaceControl;
using Elastos::Droid::View::Animation::IAnimationUtils;
using Elastos::Droid::View::Animation::CAnimationUtils;
using Elastos::Droid::View::Animation::IAnimationSet;
using Elastos::Droid::View::Animation::CAnimationSet;
using Elastos::Droid::View::Animation::EIID_IInterpolator;
using Elastos::Droid::View::Animation::IAlphaAnimation;
using Elastos::Droid::View::Animation::CAlphaAnimation;
using Elastos::Droid::View::Animation::IScaleAnimation;
using Elastos::Droid::View::Animation::CScaleAnimation;
using Elastos::Droid::View::Animation::IDecelerateInterpolator;
using Elastos::Droid::View::Animation::CDecelerateInterpolator;
using Elastos::Droid::View::Animation::EIID_IAnimation;
using Elastos::Core::CString;
using Elastos::Core::StringUtils;
using Elastos::Core::IBoolean;
using Elastos::Core::CBoolean;
using Elastos::Core::CSystem;
using Elastos::Core::ISystem;
using Elastos::IO::IWriter;
using Elastos::IO::CPrintWriter;
using Elastos::IO::IStringWriter;
using Elastos::IO::CStringWriter;
using Elastos::IO::IBufferedWriter;
using Elastos::IO::IOutputStream;
using Elastos::IO::IOutputStreamWriter;
using Elastos::IO::COutputStreamWriter;
using Elastos::IO::CBufferedWriter;
using Elastos::IO::IInputStream;
using Elastos::IO::CDataInputStream;
using Elastos::IO::IFileInputStream;
using Elastos::IO::CFileInputStream;
using Elastos::IO::CFile;
using Elastos::IO::IDataInputStream;
using Elastos::IO::IDataInput;
using Elastos::IO::IFlushable;
using Elastos::IO::ICloseable;
using Elastos::Core::IAppendable;
using Elastos::Text::IDateFormat;
using Elastos::Text::IDateFormatHelper;
using Elastos::Text::CDateFormatHelper;
using Elastos::Utility::IDate;
using Elastos::Utility::CDate;
using Elastos::Utility::Logging::Slogger;

namespace Elastos {
namespace Droid {
namespace Server {
namespace Wm {

//==============================================================================
//                  CWindowManagerService::SettingsObserver
//==============================================================================

CWindowManagerService::SettingsObserver::SettingsObserver(
    /* [in] */ CWindowManagerService* host)
    : mHost(host)
{
    AutoPtr<IHandler> h;
    CHandler::New((IHandler**)&h);
    constructor(h);
    AutoPtr<IContentResolver> resolver;
    mHost->mContext->GetContentResolver((IContentResolver**)&resolver);
    AutoPtr<IUri> uri;
    Settings::Secure::GetUriFor(ISettingsSecure::SHOW_IME_WITH_HARD_KEYBOARD, (IUri**)&uri);
    resolver->RegisterContentObserver(uri, FALSE, (IContentObserver*)this->Probe(EIID_IContentObserver));
}

ECode CWindowManagerService::SettingsObserver::OnChange(
    /* [in] */ Boolean selfChange)
{
    mHost->UpdateShowImeWithHardKeyboard();
    return NOERROR;
}


//==============================================================================
//                  CWindowManagerService::MyLowPowerModeListener
//==============================================================================

CWindowManagerService::MyLowPowerModeListener::MyLowPowerModeListener(
    /* [in] */ CWindowManagerService* host)
    : mHost(host)
{}

CAR_INTERFACE_IMPL(CWindowManagerService::MyLowPowerModeListener, Object, ILowPowerModeListener)

ECode CWindowManagerService::MyLowPowerModeListener::OnLowPowerModeChanged(
    /* [in] */ Boolean enabled)
{
    Object& lock = mHost->mWindowMapLock;
    synchronized (lock) {
        if (mHost->mAnimationsDisabled != enabled) {
            mHost->mAnimationsDisabled = enabled;
            mHost->DispatchNewAnimatorScaleLocked(NULL);
        }
    }
    return NOERROR;
}


//==============================================================================
//                  CWindowManagerService::MyAppOpsManagerOnOpChangedListener
//==============================================================================

CWindowManagerService::MyAppOpsManagerOnOpChangedListener::MyAppOpsManagerOnOpChangedListener(
    /* [in] */ CWindowManagerService* host)
    : mHost(host)
{}

CAR_INTERFACE_IMPL_2(CWindowManagerService::MyAppOpsManagerOnOpChangedListener, Object,
        IAppOpsManagerOnOpChangedListener , IAppOpsManagerOnOpChangedInternalListener)

ECode CWindowManagerService::MyAppOpsManagerOnOpChangedListener::OnOpChanged(
    /* [in] */ Int32 op,
    /* [in] */ const String& packageName)
{
    mHost->UpdateAppOpsState();
    return NOERROR;
}

ECode CWindowManagerService::MyAppOpsManagerOnOpChangedListener::OnOpChanged(
    /* [in] */ const String& op,
    /* [in] */ const String& packageName)
{
    return NOERROR;
}


//==============================================================================
//                  CWindowManagerService::LayoutFields
//==============================================================================

const Int32 CWindowManagerService::LayoutFields::SET_UPDATE_ROTATION                = 1 << 0;
const Int32 CWindowManagerService::LayoutFields::SET_WALLPAPER_MAY_CHANGE           = 1 << 1;
const Int32 CWindowManagerService::LayoutFields::SET_FORCE_HIDING_CHANGED           = 1 << 2;
const Int32 CWindowManagerService::LayoutFields::SET_ORIENTATION_CHANGE_COMPLETE    = 1 << 3;
const Int32 CWindowManagerService::LayoutFields::SET_TURN_ON_SCREEN                 = 1 << 4;
const Int32 CWindowManagerService::LayoutFields::SET_WALLPAPER_ACTION_PENDING       = 1 << 5;

CWindowManagerService::LayoutFields::LayoutFields()
    : mWallpaperForceHidingChanged(FALSE)
    , mWallpaperMayChange(FALSE)
    , mOrientationChangeComplete(TRUE)
    , mWallpaperActionPending(FALSE)
    , mDisplayHasContent(FALSE)
    , mObscureApplicationContentOnSecondaryDisplays(FALSE)
    , mPreferredRefreshRate(0)
    , mObscured(FALSE)
    , mSyswin(FALSE)
    , mScreenBrightness(-1)
    , mButtonBrightness(-1)
    , mUserActivityTimeout(-1)
    , mUpdateRotation(FALSE)
{}


//==============================================================================
//                  DragInputEventReceiver
//==============================================================================

DragInputEventReceiver::DragInputEventReceiver(
    /* [in] */ IInputChannel* inputChannel,
    /* [in] */ ILooper* looper,
    /* [in] */ CWindowManagerService* host)
    : InputEventReceiver(inputChannel, looper)
    , mHost(host)
{}

ECode DragInputEventReceiver::OnInputEvent(
    /* [in] */ IInputEvent* event)
{
    VALIDATE_NOT_NULL(event)

    Boolean handled = FALSE;
    // try {
    Int32 source;
    event->GetSource(&source);
    if (IMotionEvent::Probe(event) != NULL
            && (source & IInputDevice::SOURCE_CLASS_POINTER) != 0
            && mHost->mDragState != NULL) {
        AutoPtr<IMotionEvent> motionEvent = IMotionEvent::Probe(event);
        Boolean endDrag = FALSE;
        Float newX;
        motionEvent->GetRawX(&newX);
        Float newY;
        motionEvent->GetRawY(&newY);

        Int32 action;
        motionEvent->GetAction(&action);
        switch (action) {
        case IMotionEvent::ACTION_DOWN: {
            if (CWindowManagerService::DEBUG_DRAG) {
                Slogger::W(CWindowManagerService::TAG, "Unexpected ACTION_DOWN in drag layer");
            }
        } break;

        case IMotionEvent::ACTION_MOVE: {
            AutoLock lock(mHost->mWindowMapLock);
            // move the surface and tell the involved window(s) where we are
            mHost->mDragState->NotifyMoveLw(newX, newY);
        } break;

        case IMotionEvent::ACTION_UP: {
            if (CWindowManagerService::DEBUG_DRAG) Slogger::D(CWindowManagerService::TAG, "Got UP on move channel; dropping at %f,%f",
                    newX, newY);
            AutoLock lock(mHost->mWindowMapLock);
            endDrag = mHost->mDragState->NotifyDropLw(newX, newY);
        } break;

        case IMotionEvent::ACTION_CANCEL: {
            if (CWindowManagerService::DEBUG_DRAG) Slogger::D(CWindowManagerService::TAG, "Drag cancelled!");
            endDrag = TRUE;
        } break;
        }

        if (endDrag) {
            if (CWindowManagerService::DEBUG_DRAG) Slogger::D(CWindowManagerService::TAG, "Drag ended; tearing down state");
            // tell all the windows that the drag has ended
            AutoLock lock(mHost->mWindowMapLock);
            mHost->mDragState->EndDragLw();
        }

        handled = TRUE;
    }
    // } catch (Exception e) {
    //     Slog.e(TAG, "Exception caught by drag handleMotion", e);
    // } finally {
    FinishInputEvent(event, handled);
    // }
    return NOERROR;
}


//==============================================================================
//                  CWindowManagerService::WindowManagerServiceCreator
//==============================================================================

CWindowManagerService::WindowManagerServiceCreator::WindowManagerServiceCreator(
    /* [in] */ IContext* context,
    /* [in] */ CInputManagerService* im,
    /* [in] */ Boolean haveInputMethods,
    /* [in] */ Boolean showBootMsgs,
    /* [in] */ Boolean onlyCore)
    : mContext(context)
    // , mIm(im)
    , mHaveInputMethods(haveInputMethods)
    , mShowBootMsgs(showBootMsgs)
    , mOnlyCore(onlyCore)
{}

ECode CWindowManagerService::WindowManagerServiceCreator::Run()
{
    //TODO: CInputManagerService
    return CWindowManagerService::NewByFriend(mContext, 0/*(Handle64)mIm.Get()*/,
            mHaveInputMethods, mShowBootMsgs, mOnlyCore, (CWindowManagerService**)&mInstance);
}


//==============================================================================
//                  CWindowManagerService::PolicyInitializer
//==============================================================================

CWindowManagerService::PolicyInitializer::PolicyInitializer(
    /* [in] */ CWindowManagerService* wmService)
    : mHost(wmService)
{}

ECode CWindowManagerService::PolicyInitializer::Run()
{
    // TODO:
    // WindowManagerPolicyThread.set(Thread::GetCurrentThread(), Looper::GetMyLooper());

    FAIL_RETURN(mHost->mPolicy->Init(mHost->mContext,
            (IIWindowManager*)mHost->Probe(EIID_IIWindowManager),
            (IWindowManagerPolicyWindowManagerFuncs*)mHost->Probe(EIID_IWindowManagerPolicyWindowManagerFuncs)));
    Int32 layer;
    mHost->mPolicy->GetAboveUniverseLayer(&layer);
    mHost->mAnimator->mAboveUniverseLayer = layer * TYPE_LAYER_MULTIPLIER
            + TYPE_LAYER_OFFSET;
    return NOERROR;
}


//==============================================================================
//                  CWindowManagerService::RotationWatcherDeathRecipint
//==============================================================================

CAR_INTERFACE_IMPL(CWindowManagerService::RotationWatcherDeathRecipint, Object, IProxyDeathRecipient)

CWindowManagerService::RotationWatcherDeathRecipint::RotationWatcherDeathRecipint(
    /* [in] */ CWindowManagerService* owner,
    /* [in] */ IBinder* watcherBinder)
    : mOwner(owner)
    , mWatcherBinder(watcherBinder)
{}

ECode CWindowManagerService::RotationWatcherDeathRecipint::ProxyDied()
{
    Object& lock = mOwner->mWindowMapLock;
    synchronized (lock) {
        List<AutoPtr<RotationWatcher> >::Iterator iter = mOwner->mRotationWatchers.Begin();
        while (iter != mOwner->mRotationWatchers.End()) {
            AutoPtr<IBinder> b = IBinder::Probe((*iter)->mWatcher);
            if (mWatcherBinder == b) {
                AutoPtr<RotationWatcher> removed = *iter;
                AutoPtr<IProxy> proxy = (IProxy*)removed->mWatcher->Probe(EIID_IProxy);
                if (proxy != NULL) {
                    Boolean res;
                    proxy->UnlinkToDeath(this, 0, &res);
                }
                List<AutoPtr<RotationWatcher> >::Iterator temp = iter;
                iter++;
                mOwner->mRotationWatchers.Erase(temp);
            }
            else
                ++iter;
        }
    }
    return NOERROR;
}


//==============================================================================
//                  CWindowManagerService::LocalBroadcastReceiver
//==============================================================================

ECode CWindowManagerService::LocalBroadcastReceiver::OnReceive(
    /* [in] */ IContext* context,
    /* [in] */ IIntent* intent)
{
    String action;
    intent->GetAction(&action);
    if (IDevicePolicyManager::ACTION_DEVICE_POLICY_MANAGER_STATE_CHANGED.Equals(action)) {
        Boolean result;
        mHost->mKeyguardDisableHandler->SendEmptyMessage(
            KeyguardDisableHandler::KEYGUARD_POLICY_CHANGED, &result);
    }
    return NOERROR;
}


//==============================================================================
//                  CWindowManagerService::SecurelyOnKeyguardExitResult
//==============================================================================

CAR_INTERFACE_IMPL(CWindowManagerService::SecurelyOnKeyguardExitResult, Object, IOnKeyguardExitResult);

ECode CWindowManagerService::SecurelyOnKeyguardExitResult::OnKeyguardExitResult(
    /* [in] */ Boolean success)
{
    // try {
    return mCallback->OnKeyguardExitResult(success);
    // } catch (RemoteException e) {
    //     // Client has died, we don't care.
    // }
}


//==============================================================================
//                  CWindowManagerService::LocalService
//==============================================================================

CWindowManagerService::LocalService::LocalService(
    /* [in] */ CWindowManagerService* host)
    : mHost(host)
{}

CAR_INTERFACE_IMPL(CWindowManagerService::LocalService, Object, IWindowManagerInternal)

ECode CWindowManagerService::LocalService::RequestTraversalFromDisplayManager()
{
    return mHost->RequestTraversal();
}

ECode CWindowManagerService::LocalService::SetMagnificationSpec(
    /* [in] */ IMagnificationSpec* spec)
{
    Object& lock = mHost->mWindowMapLock;
    synchronized (lock) {
        if (mHost->mAccessibilityController != NULL) {
            mHost->mAccessibilityController->SetMagnificationSpecLocked(spec);
        }
        else {
            Slogger::E("CWindowManagerService::LocalService", "Magnification callbacks not set!");
            return E_ILLEGAL_STATE_EXCEPTION;
        }
    }
    if (Binder::GetCallingPid() != Process::MyPid()) {
        spec->Recycle();
    }
    return NOERROR;
}

ECode CWindowManagerService::LocalService::GetCompatibleMagnificationSpecForWindow(
    /* [in] */ IBinder* windowToken,
    /* [out] */ IMagnificationSpec** _spec)
{
    VALIDATE_NOT_NULL(_spec)
    *_spec = NULL;

    Object& lock = mHost->mWindowMapLock;
    synchronized (lock) {
        AutoPtr<WindowState> windowState;
        HashMap<AutoPtr<IBinder>, AutoPtr<WindowState> >::Iterator it = mHost->mWindowMap.Find(windowToken);
        if (it != mHost->mWindowMap.End() && it->mSecond != NULL) {
            windowState = it->mSecond;
        }
        if (windowState == NULL) {
            *_spec = NULL;
            return NOERROR;
        }
        AutoPtr<IMagnificationSpec> spec;
        if (mHost->mAccessibilityController != NULL) {
            spec = mHost->mAccessibilityController->GetMagnificationSpecForWindowLocked(windowState);
        }
        Boolean isNop;
        if ((spec == NULL || (spec->IsNop(&isNop), isNop)) && windowState->mGlobalScale == 1.0f) {
            *_spec = NULL;
            return NOERROR;
        }
        AutoPtr<IMagnificationSpecHelper> helper;
        CMagnificationSpecHelper::AcquireSingleton((IMagnificationSpecHelper**)&helper);
        if (spec == NULL) {
            helper->Obtain((IMagnificationSpec**)&spec);
        }
        else {
            AutoPtr<IMagnificationSpec> temp = spec;
            spec = NULL;
            helper->Obtain(temp, (IMagnificationSpec**)&spec);
        }
        Float scale;
        spec->GetScale(&scale);
        scale *= windowState->mGlobalScale;
        spec->SetScale(scale);
        *_spec = spec;
        REFCOUNT_ADD(*_spec)
    }
    return NOERROR;
}

ECode CWindowManagerService::LocalService::SetMagnificationCallbacks(
    /* [in] */ IMagnificationCallbacks* callbacks)
{
    Object& lock = mHost->mWindowMapLock;
    synchronized (lock) {
        if (mHost->mAccessibilityController == NULL) {
            mHost->mAccessibilityController = new AccessibilityController(mHost);
        }
        mHost->mAccessibilityController->SetMagnificationCallbacksLocked(callbacks);
        if (!mHost->mAccessibilityController->HasCallbacksLocked()) {
            mHost->mAccessibilityController = NULL;
        }
    }
    return NOERROR;
}

ECode CWindowManagerService::LocalService::SetWindowsForAccessibilityCallback(
    /* [in] */ IWindowsForAccessibilityCallback* callback)
{
    Object& lock = mHost->mWindowMapLock;
    synchronized (lock) {
        if (mHost->mAccessibilityController == NULL) {
            mHost->mAccessibilityController = new AccessibilityController(mHost);
        }
        mHost->mAccessibilityController->SetWindowsForAccessibilityCallback(callback);
        if (!mHost->mAccessibilityController->HasCallbacksLocked()) {
            mHost->mAccessibilityController = NULL;
        }
    }
    return NOERROR;
}

ECode CWindowManagerService::LocalService::SetInputFilter(
    /* [in] */ IIInputFilter* filter)
{
    mHost->mInputManager->SetInputFilter(filter);
    return NOERROR;
}

ECode CWindowManagerService::LocalService::GetFocusedWindowToken(
    /* [out] */ IBinder** binder)
{
    VALIDATE_NOT_NULL(binder)
    *binder = NULL;

    Object& lock = mHost->mWindowMapLock;
    synchronized (lock) {
        AutoPtr<WindowState> windowState = mHost->GetFocusedWindowLocked();
        if (windowState != NULL) {
            *binder = IBinder::Probe(windowState->mClient);
            REFCOUNT_ADD(*binder)
        }
    }
    return NOERROR;
}

ECode CWindowManagerService::LocalService::IsKeyguardLocked(
    /* [out] */ Boolean* isLocked)
{
    VALIDATE_NOT_NULL(isLocked)
    return mHost->IsKeyguardLocked(isLocked);
}

ECode CWindowManagerService::LocalService::ShowGlobalActions()
{
    mHost->ShowGlobalActions();
    return NOERROR;
}

ECode CWindowManagerService::LocalService::GetWindowFrame(
    /* [in] */ IBinder* token,
    /* [in] */ IRect* outBounds)
{
    Object& lock = mHost->mWindowMapLock;
    synchronized (lock) {
        AutoPtr<WindowState> windowState;
        HashMap<AutoPtr<IBinder>, AutoPtr<WindowState> >::Iterator it = mHost->mWindowMap.Find(token);
        if (it != mHost->mWindowMap.End() && it->mSecond != NULL) {
            windowState = it->mSecond;
        }
        if (windowState != NULL) {
            outBounds->Set(windowState->mFrame);
        }
        else {
            outBounds->SetEmpty();
        }
    }
    return NOERROR;
}

ECode CWindowManagerService::LocalService::WaitForAllWindowsDrawn(
    /* [in] */ IRunnable* callback,
    /* [in] */ Int64 timeout)
{
    Object& lock = mHost->mWindowMapLock;
    synchronized (lock) {
        mHost->mWaitingForDrawnCallback = callback;
        AutoPtr<WindowList> windows = mHost->GetDefaultWindowListLocked();
        WindowList::ReverseIterator rit = windows->RBegin();
        for (; rit != windows->REnd(); ++rit) {
            AutoPtr<WindowState> win = *rit;
            Boolean isVisible, isHiding;
            if ((win->IsVisibleLw(&isVisible), isVisible)
                    && (win->mAppToken != NULL || (mHost->mPolicy->IsForceHiding(win->mAttrs, &isHiding), isHiding))) {
                win->mWinAnimator->mDrawState = WindowStateAnimator::DRAW_PENDING;
                // Force add to mResizingWindows.
                win->mLastContentInsets->Set(-1, -1, -1, -1);
                mHost->mWaitingForDrawn.PushBack(win);
            }
        }
        mHost->RequestTraversalLocked();
    }
    mHost->mH->RemoveMessages(CWindowManagerService::H::WAITING_FOR_DRAWN_TIMEOUT);
    if (mHost->mWaitingForDrawn.IsEmpty()) {
        callback->Run();
    }
    else {
        Boolean result;
        mHost->mH->SendEmptyMessageDelayed(
                CWindowManagerService::H::WAITING_FOR_DRAWN_TIMEOUT, timeout, &result);
        mHost->CheckDrawnWindowsLocked();
    }
    return NOERROR;
}


//==============================================================================
//                  CWindowManagerService
//==============================================================================

const String CWindowManagerService::TAG("CWindowManager");
const Boolean CWindowManagerService::DEBUG;
const Boolean CWindowManagerService::DEBUG_ADD_REMOVE;
const Boolean CWindowManagerService::DEBUG_FOCUS;
const Boolean CWindowManagerService::DEBUG_FOCUS_LIGHT;
const Boolean CWindowManagerService::DEBUG_ANIM;
const Boolean CWindowManagerService::DEBUG_LAYOUT;
const Boolean CWindowManagerService::DEBUG_RESIZE;
const Boolean CWindowManagerService::DEBUG_LAYERS;
const Boolean CWindowManagerService::DEBUG_INPUT;
const Boolean CWindowManagerService::DEBUG_INPUT_METHOD;
const Boolean CWindowManagerService::DEBUG_VISIBILITY;
const Boolean CWindowManagerService::DEBUG_WINDOW_MOVEMENT;
const Boolean CWindowManagerService::DEBUG_TOKEN_MOVEMENT;
const Boolean CWindowManagerService::DEBUG_ORIENTATION;
const Boolean CWindowManagerService::DEBUG_APP_ORIENTATION;
const Boolean CWindowManagerService::DEBUG_CONFIGURATION;
const Boolean CWindowManagerService::DEBUG_APP_TRANSITIONS;
const Boolean CWindowManagerService::DEBUG_STARTING_WINDOW;
const Boolean CWindowManagerService::DEBUG_REORDER;
const Boolean CWindowManagerService::DEBUG_WALLPAPER;
const Boolean CWindowManagerService::DEBUG_WALLPAPER_LIGHT;
const Boolean CWindowManagerService::DEBUG_DRAG;
const Boolean CWindowManagerService::DEBUG_SCREEN_ON;
const Boolean CWindowManagerService::DEBUG_SCREENSHOT;
const Boolean CWindowManagerService::DEBUG_BOOT;
const Boolean CWindowManagerService::DEBUG_LAYOUT_REPEATS;
const Boolean CWindowManagerService::DEBUG_SURFACE_TRACE;
const Boolean CWindowManagerService::DEBUG_WINDOW_TRACE;
const Boolean CWindowManagerService::DEBUG_TASK_MOVEMENT;
const Boolean CWindowManagerService::DEBUG_STACK;
const Boolean CWindowManagerService::DEBUG_DISPLAY;
const Boolean CWindowManagerService::SHOW_SURFACE_ALLOC;
const Boolean CWindowManagerService::SHOW_TRANSACTIONS;
const Boolean CWindowManagerService::SHOW_LIGHT_TRANSACTIONS;
const Boolean CWindowManagerService::HIDE_STACK_CRAWLS;
const Int32 CWindowManagerService::LAYOUT_REPEAT_THRESHOLD;
const Boolean CWindowManagerService::PROFILE_ORIENTATION;
const Boolean CWindowManagerService::localLOGV;
const Int32 CWindowManagerService::TYPE_LAYER_MULTIPLIER;
const Int32 CWindowManagerService::TYPE_LAYER_OFFSET;
const Int32 CWindowManagerService::WINDOW_LAYER_MULTIPLIER;
const Int32 CWindowManagerService::LAYER_OFFSET_DIM;
const Int32 CWindowManagerService::LAYER_OFFSET_BLUR;
const Int32 CWindowManagerService::LAYER_OFFSET_FOCUSED_STACK;
const Int32 CWindowManagerService::LAYER_OFFSET_THUMBNAIL;
const Int32 CWindowManagerService::FREEZE_LAYER;
const Int32 CWindowManagerService::MASK_LAYER;
const Int32 CWindowManagerService::MAX_ANIMATION_DURATION;
const Int32 CWindowManagerService::DEFAULT_FADE_IN_OUT_DURATION;
const Int32 CWindowManagerService::WINDOW_FREEZE_TIMEOUT_DURATION;
const Boolean CWindowManagerService::CUSTOM_SCREEN_ROTATION;
const Int64 CWindowManagerService::DEFAULT_INPUT_DISPATCHING_TIMEOUT_NANOS;
const Int32 CWindowManagerService::BOOT_ANIMATION_POLL_INTERVAL;
const String CWindowManagerService::BOOT_ANIMATION_SERVICE("bootanim");
const Float CWindowManagerService::STACK_WEIGHT_MIN;
const Float CWindowManagerService::STACK_WEIGHT_MAX;
const Int32 CWindowManagerService::UPDATE_FOCUS_NORMAL;
const Int32 CWindowManagerService::UPDATE_FOCUS_WILL_ASSIGN_LAYERS;
const Int32 CWindowManagerService::UPDATE_FOCUS_PLACING_SURFACES;
const Int32 CWindowManagerService::UPDATE_FOCUS_WILL_PLACE_SURFACES;
const Int64 CWindowManagerService::WALLPAPER_TIMEOUT;
const Int64 CWindowManagerService::WALLPAPER_TIMEOUT_RECOVERY;
const Int32 CWindowManagerService::ADJUST_WALLPAPER_LAYERS_CHANGED;
const Int32 CWindowManagerService::ADJUST_WALLPAPER_VISIBILITY_CHANGED;
const Int32 CWindowManagerService::INPUT_DEVICES_READY_FOR_SAFE_MODE_DETECTION_TIMEOUT_MILLIS;
const String CWindowManagerService::SYSTEM_SECURE("ro.secure");
const String CWindowManagerService::SYSTEM_DEBUGGABLE("ro.debuggable");
const String CWindowManagerService::DENSITY_OVERRIDE("ro.config.density_override");
const String CWindowManagerService::SIZE_OVERRIDE("ro.config.size_override");
const String CWindowManagerService::SYSTEM_DEFAULT_ROTATION("ro.sf.default_rotation");
const Int32 CWindowManagerService::MAX_SCREENSHOT_RETRIES;
const Int32 CWindowManagerService::SYSTEM_UI_FLAGS_LAYOUT_STABLE_FULLSCREEN;
const String CWindowManagerService::PROPERTY_EMULATOR_CIRCULAR("ro.emulator.circular");

static List< AutoPtr<WindowState> >::Iterator Find(
    /* [in] */ List< AutoPtr<WindowState> >& windows,
    /* [in] */ WindowState* win)
{
    List< AutoPtr<WindowState> >::Iterator it = windows.Begin();
    for (; it != windows.End(); ++it) {
        if ((*it).Get() == win) {
            break;
        }
    }
    return it;
}


CWindowManagerService::CWindowManagerService()
    : mCurrentUserId(0)
    , mHaveInputMethods(FALSE)
    , mHasPermanentDpad(FALSE)
    , mAllowBootMessages(FALSE)
    , mLimitedAlphaCompositing(FALSE)
    , mFocusedStackLayer(0)
    , mDisplayReady(FALSE)
    , mSafeMode(FALSE)
    , mDisplayEnabled(FALSE)
    , mSystemBooted(FALSE)
    , mForceDisplayEnabled(FALSE)
    , mShowingBootMessages(FALSE)
    , mBootAnimationStopped(FALSE)
    , mRotation(0)
    , mForcedAppOrientation(IActivityInfo::SCREEN_ORIENTATION_UNSPECIFIED)
    , mAltOrientation(FALSE)
    , mDeferredRotationPauseCount(0)
    , mSystemDecorLayer(0)
    , mTraversalScheduled(FALSE)
    , mDisplayFrozen(FALSE)
    , mDisplayFreezeTime(0)
    , mLastDisplayFreezeDuration(0)
    , mWaitingForConfig(FALSE)
    , mWindowsFreezingScreen(FALSE)
    , mClientFreezingScreen(FALSE)
    , mAppsFreezingScreen(0)
    , mLastWindowForcedOrientation(IActivityInfo::SCREEN_ORIENTATION_UNSPECIFIED)
    , mLayoutSeq(0)
    , mLastStatusBarVisibility(0)
    , mFocusMayChange(FALSE)
    , mStartingIconInTransition(FALSE)
    , mSkipAppTransitionAnimation(FALSE)
    , mIsTouchDevice(FALSE)
    , mInputMethodTargetWaitingAnim(FALSE)
    , mInputMethodAnimLayerAdjustment(0)
    , mHardKeyboardAvailable(FALSE)
    , mShowImeWithHardKeyboard(FALSE)
    , mWallpaperAnimLayerAdjustment(0)
    , mLastWallpaperX(-1)
    , mLastWallpaperY(-1)
    , mLastWallpaperXStep(-1)
    , mLastWallpaperYStep(-1)
    , mLastWallpaperDisplayOffsetX(Elastos::Core::Math::INT32_MIN_VALUE)
    , mLastWallpaperDisplayOffsetY(Elastos::Core::Math::INT32_MIN_VALUE)
    , mLastWallpaperTimeoutTime(0)
    , mAnimateWallpaperWithTarget(FALSE)
    , mWindowAnimationScaleSetting(1.0f)
    , mTransitionAnimationScaleSetting(1.0f)
    , mAnimatorDurationScaleSetting(1.0f)
    , mAnimationsDisabled(FALSE)
    , mTurnOnScreen(FALSE)
    , mExitAnimId(0)
    , mEnterAnimId(0)
    , mAnimationScheduled(FALSE)
    , mInTouchMode(TRUE)
    , mCompatibleScreenScale(0)
    , mOnlyCore(FALSE)
    , mKeyguardWaitingForActivityDrawn(FALSE)
    , mTransactionSequence(0)
    , mLayoutRepeatCount(0)
    , mWindowsChanged(FALSE)
    , mEventDispatchingEnabled(FALSE)
    , mInLayout(FALSE)
{
    mCurrentProfileIds = ArrayOf<Int32>::Alloc(1);
    (*mCurrentProfileIds)[0] = IUserHandle::USER_OWNER;

    CArraySet::New((IArraySet**)&mSessions);
    CArraySet::New((IArraySet**)&mPendingStacksRemove);

    CSparseArray::New((ISparseArray**)&mScreenCaptureDisabled);
    CSparseArray::New(2, (ISparseArray**)&mDisplayContents);

    CSparseArray::New((ISparseArray**)&mTaskIdToTask);
    CSparseArray::New((ISparseArray**)&mStackIdToStack);

    mBroadcastReceiver = new LocalBroadcastReceiver(this);

    AutoPtr<IPolicyManager> pm;
    CPolicyManager::AcquireSingleton((IPolicyManager**)&pm);
    ASSERT_SUCCEEDED(pm->MakeNewWindowManager((IWindowManagerPolicy**)&mPolicy));

    mPendingRemoveTmp = ArrayOf<WindowState*>::Alloc(20);
    mRebuildTmp = ArrayOf<WindowState*>::Alloc(20);
    mTmpFloats = ArrayOf<Float>::Alloc(9);

    CRect::New((IRect**)&mTmpContentRect);

    CConfiguration::New((IConfiguration**)&mCurConfiguration);

    CRect::New((IRect**)&mScreenRect);

    CDisplayMetrics::New((IDisplayMetrics**)&mDisplayMetrics);
    CDisplayMetrics::New((IDisplayMetrics**)&mRealDisplayMetrics);
    CDisplayMetrics::New((IDisplayMetrics**)&mTmpDisplayMetrics);
    CDisplayMetrics::New((IDisplayMetrics**)&mCompatDisplayMetrics);

    mH = new H(this);

    AutoPtr<IChoreographerHelper> helper;
    CChoreographerHelper::AcquireSingleton((IChoreographerHelper**)&helper);
    helper->GetInstance((IChoreographer**)&mChoreographer);

    mInnerFields = new LayoutFields();

    CConfiguration::New((IConfiguration**)&mTempConfiguration);
    mInputMonitor = new InputMonitor(this);
}

CAR_INTERFACE_IMPL_3(CWindowManagerService, Object, IIWindowManager, IWindowManagerPolicyWindowManagerFuncs, IBinder)

CAR_OBJECT_IMPL(CWindowManagerService)

AutoPtr<CWindowManagerService> CWindowManagerService::Main(
    /* [in] */ IContext* context,
    /* [in] */ CInputManagerService* im,
    /* [in] */ Boolean haveInputMethods,
    /* [in] */ Boolean showBootMsgs,
    /* [in] */ Boolean onlyCore)
{
    AutoPtr<WindowManagerServiceCreator> runnable =
            new WindowManagerServiceCreator(context, im, haveInputMethods, showBootMsgs, onlyCore);
    Boolean result;
    DisplayThread::GetHandler()->RunWithScissors((IRunnable*)runnable.Get(), 0, &result);
    return runnable->mInstance;
}

void CWindowManagerService::InitPolicy()
{
    AutoPtr<IRunnable> runnable = new PolicyInitializer(this);
    Boolean result;
    UiThread::GetHandler()->RunWithScissors(runnable, 0, &result);
}

ECode CWindowManagerService::constructor(
    /* [in] */ IContext* context,
    /* [in] */ Handle64 inputManager,
    /* [in] */ Boolean haveInputMethods,
    /* [in] */ Boolean showBootMsgs,
    /* [in] */ Boolean onlyCore)
{
    mContext = context;
    mHaveInputMethods = haveInputMethods;
    mAllowBootMessages = showBootMsgs;
    mOnlyCore = onlyCore;
    AutoPtr<IResources> resources;
    ASSERT_SUCCEEDED(mContext->GetResources((IResources**)&resources))
    resources->GetBoolean(Elastos::Droid::R::bool_::config_sf_limitedAlpha, &mLimitedAlphaCompositing);
    resources->GetBoolean(Elastos::Droid::R::bool_::config_hasPermanentDpad, &mHasPermanentDpad);
    mInputManager = (CInputManagerService*)inputManager; // Must be before createDisplayContentLocked.
    mDisplayManagerInternal = (IDisplayManagerInternal*)LocalServices::GetService(EIID_IDisplayManagerInternal).Get();
    mDisplaySettings = new DisplaySettings(context);
    mDisplaySettings->ReadSettingsLocked();

    LocalServices::AddService(EIID_IWindowManagerPolicy, mPolicy);

    AutoPtr<IInputChannel> inputChannel;
    mInputManager->MonitorInput(TAG, (IInputChannel**)&inputChannel);
    mPointerEventDispatcher = new PointerEventDispatcher(inputChannel);

    ASSERT_SUCCEEDED(CSurfaceSession::New((ISurfaceSession**)&mFxSession))
    ASSERT_SUCCEEDED(context->GetSystemService(IContext::DISPLAY_SERVICE, (IInterface**)&mDisplayManager))
    AutoPtr< ArrayOf<IDisplay*> > displays;
    mDisplayManager->GetDisplays((ArrayOf<IDisplay*>**)&displays);
    for (Int32 i = 0; i < displays->GetLength(); ++i) {
        AutoPtr<IDisplay> display = (*displays)[i];
        CreateDisplayContentLocked(display);
    }
    //ActionsCode(wh, NEW_FEATURE : support deviece default rotation)
    // if(IActionsConfig::ACTIONS_FEATURE_SET_USER_DEFAULT_ROTATION_DEFAULT_ON >= 1){
    //     AutoPtr<ISystemProperties> sysProp;
    //     CSystemProperties::AcquireSingleton((ISystemProperties**)&sysProp);
    //     sysProp->GetInt32(SYSTEM_DEFAULT_ROTATION, 0, &mRotation);
    // }
    mKeyguardDisableHandler = new KeyguardDisableHandler(mContext, mPolicy);

    ASSERT_SUCCEEDED(context->GetSystemService(IContext::POWER_SERVICE, (IInterface**)&mPowerManager))
    mPowerManagerInternal = (IPowerManagerInternal*)LocalServices::GetService(EIID_IPowerManagerInternal).Get();
    AutoPtr<ILowPowerModeListener> powerListener = new MyLowPowerModeListener(this);
    mPowerManagerInternal->RegisterLowPowerModeObserver(powerListener);
    mPowerManagerInternal->GetLowPowerModeEnabled(&mAnimationsDisabled);
    mPowerManager->NewWakeLock(IPowerManager::PARTIAL_WAKE_LOCK,
            String("SCREEN_FROZEN"), (IPowerManagerWakeLock**)&mScreenFrozenLock);
    mScreenFrozenLock->SetReferenceCounted(FALSE);

    mAppTransition = new AppTransition(context, mH);

    mActivityManager = ActivityManagerNative::GetDefault();
    mBatteryStats = CBatteryStatsService::GetService();
    ASSERT_SUCCEEDED(context->GetSystemService(IContext::APP_OPS_SERVICE, (IInterface**)&mAppOps))
    AutoPtr<IAppOpsManagerOnOpChangedListener> opListener = new MyAppOpsManagerOnOpChangedListener(this);
    mAppOps->StartWatchingMode(IAppOpsManager::OP_SYSTEM_ALERT_WINDOW, String(NULL), opListener);
    mAppOps->StartWatchingMode(IAppOpsManager::OP_TOAST_WINDOW, String(NULL), opListener);

    // Get persisted window scale setting
    AutoPtr<IContentResolver> resolver;
    context->GetContentResolver((IContentResolver**)&resolver);
    Settings::Global::GetFloat(resolver, ISettingsGlobal::WINDOW_ANIMATION_SCALE,
            mWindowAnimationScaleSetting, &mWindowAnimationScaleSetting);
    Settings::Global::GetFloat(resolver, ISettingsGlobal::TRANSITION_ANIMATION_SCALE,
            mTransitionAnimationScaleSetting, &mTransitionAnimationScaleSetting);
    Float scaleSetting;
    Settings::Global::GetFloat(resolver, ISettingsGlobal::ANIMATOR_DURATION_SCALE,
            mAnimatorDurationScaleSetting, &scaleSetting);
    SetAnimatorDurationScale(scaleSetting);

    // Track changes to DevicePolicyManager state so we can enable/disable keyguard.
    AutoPtr<IIntentFilter> filter;
    CIntentFilter::New((IIntentFilter**)&filter);
    filter->AddAction(IDevicePolicyManager::ACTION_DEVICE_POLICY_MANAGER_STATE_CHANGED);
    AutoPtr<IIntent> intent;
    mContext->RegisterReceiver(mBroadcastReceiver, filter, (IIntent**)&intent);

    mSettingsObserver = new SettingsObserver(this);
    UpdateShowImeWithHardKeyboard();

    mPowerManager->NewWakeLock(IPowerManager::SCREEN_BRIGHT_WAKE_LOCK | IPowerManager::ON_AFTER_RELEASE,
            TAG, (IPowerManagerWakeLock**)&mHoldingScreenWakeLock);
    mHoldingScreenWakeLock->SetReferenceCounted(FALSE);

    mAnimator = new WindowAnimator(this);

    AutoPtr<IWindowManagerInternal> wmInternal = new LocalService(this);
    LocalServices::AddService(EIID_IWindowManagerInternal, wmInternal);
    InitPolicy();

    // Add ourself to the Watchdog monitors.
    // Watchdog.getInstance().addMonitor(this);

    AutoPtr<ISurfaceControlHelper> scHelper;
    CSurfaceControlHelper::AcquireSingleton((ISurfaceControlHelper**)&scHelper);
    scHelper->OpenTransaction();
    // try {
    CreateWatermarkInTransaction();
    AutoPtr<IDisplay> d = GetDefaultDisplayContentLocked()->GetDisplay();
    mFocusedStackFrame = new FocusedStackFrame(d, mFxSession);
    // } finally {
    scHelper->CloseTransaction();
    // }

    ShowCircularDisplayMaskIfNeeded();
    ShowEmulatorDisplayOverlayIfNeeded();

    return NOERROR;
}

AutoPtr<InputMonitor> CWindowManagerService::GetInputMonitor()
{
    return mInputMonitor;
}

void CWindowManagerService::PlaceWindowAfter(
    /* [in] */ WindowState* pos,
    /* [in] */ WindowState* window)
{
    AutoPtr<WindowList> windows = pos->GetWindowList();
    WindowList::Iterator it = Find(*windows, pos);
    if (DEBUG_FOCUS || DEBUG_WINDOW_MOVEMENT || DEBUG_ADD_REMOVE) {
        WindowList::Iterator winIt = windows->Begin();
        Int32 i = 0;
        for (; winIt != windows->End(); ++winIt, i++) {
            if ((*winIt).Get() == pos) {
                break;
            }
        }
        Slogger::V(TAG, "Adding window %p at %d of %d (after %p)",
                window, i, windows->GetSize(), pos);
    }
    if (it == windows->End()) {
        windows->PushBack(window);
    }
    else {
        windows->Insert(++it, window);
    }
    mWindowsChanged = TRUE;
}

void CWindowManagerService::PlaceWindowBefore(
    /* [in] */ WindowState* pos,
    /* [in] */ WindowState* window)
{
    AutoPtr<WindowList> windows = pos->GetWindowList();
    WindowList::Iterator it = Find(*windows, pos);
    if (DEBUG_FOCUS || DEBUG_WINDOW_MOVEMENT || DEBUG_ADD_REMOVE) {
        WindowList::Iterator winIt = windows->Begin();
        Int32 i = 0;
        for (; winIt != windows->End(); ++winIt, i++) {
            if ((*winIt).Get() == pos) {
                break;
            }
        }
        Slogger::V(TAG, "Adding window %p at %d of %d (before %p)",
                window, i, windows->GetSize(), pos);
    }
    if (it == windows->End()) {
        Slogger::W(TAG, "placeWindowBefore: Unable to find %d, in windows", pos);
        it = windows->Begin();
    }
    windows->Insert(it, window);
    mWindowsChanged = TRUE;
}

List<AutoPtr<WindowState> >::Iterator
CWindowManagerService::FindIteratorBasedOnAppTokens(
    /* [in] */ WindowState* win)
{
    AutoPtr<WindowList> windows = win->GetWindowList();
    WindowList::ReverseIterator rit = windows->RBegin();
    for(; rit != windows->REnd(); ++rit) {
        AutoPtr<WindowState> wentry = *rit;
        if(wentry->mAppToken == win->mAppToken) {
            return (++rit).GetBase();
        }
    }
    return windows->End();
}

AutoPtr<List<AutoPtr<WindowState> > > CWindowManagerService::GetTokenWindowsOnDisplay(
    /* [in] */ WindowToken* token,
    /* [in] */ DisplayContent* displayContent)
{
    AutoPtr<WindowList> windowList = new List<AutoPtr<WindowState> >();
    List<AutoPtr<WindowState> >::Iterator it = token->mWindows.Begin();
    for (; it != token->mWindows.End(); ++it) {
        AutoPtr<WindowState> win = *it;
        if (win->GetDisplayContent().Get() == displayContent) {
            windowList->PushBack(win);
        }
    }
    return windowList;
}

List<AutoPtr<WindowState> >::Iterator CWindowManagerService::IndexOfWinInWindowList(
    /* [in] */ WindowState* targetWin,
    /* [in] */ List<AutoPtr<WindowState> >& windows)
{
    List<AutoPtr<WindowState> >::Iterator it = windows.Begin();
    for(; it != windows.End(); it++) {
        AutoPtr<WindowState> w = *it;
        if (w.Get() == targetWin) {
            return it;
        }
        if (!w->mChildWindows.IsEmpty()) {
            if(IndexOfWinInWindowList(targetWin, w->mChildWindows) != w->mChildWindows.End()) {
                return it;
            }
        }
    }
    return windows.End();
}

List<AutoPtr<WindowState> >::Iterator CWindowManagerService::AddAppWindowToListLocked(
    /* [in] */ WindowState* win)
{
    AutoPtr<IIWindow> client = win->mClient;
    AutoPtr<WindowToken> token = win->mToken;
    AutoPtr<DisplayContent> displayContent = win->GetDisplayContent();
    if (displayContent == NULL) {
        // It doesn't matter this display is going away.
        return 0;
    }

    AutoPtr<WindowList> windows = win->GetWindowList();
    AutoPtr<WindowList> tokenWindowList = GetTokenWindowsOnDisplay(token, displayContent);
    WindowList::Iterator tokenWindowsIt = tokenWindowList->Begin();
    if (tokenWindowList->Begin() != tokenWindowList->End()) {
        // If this application has existing windows, we
        // simply place the new window on top of them... but
        // keep the starting window on top.
        Int32 type;
        if (win->mAttrs->GetType(&type), type == IWindowManagerLayoutParams::TYPE_BASE_APPLICATION) {
            // Base windows go behind everything else.
            AutoPtr<WindowState> lowestWindow = *(tokenWindowList->Begin());
            PlaceWindowBefore(lowestWindow, win);
            tokenWindowsIt = IndexOfWinInWindowList(lowestWindow, token->mWindows);
        }
        else {
            AutoPtr<AppWindowToken> atoken = win->mAppToken;
            AutoPtr<WindowState> lastWindow = *(tokenWindowList->End());
            if (atoken != NULL && lastWindow == atoken->mStartingWindow) {
                PlaceWindowBefore(lastWindow, win);
                tokenWindowsIt = IndexOfWinInWindowList(lastWindow, token->mWindows);
            }
            else {
                List<AutoPtr<WindowState> >::Iterator newIt = FindIteratorBasedOnAppTokens(win);
                //there is a window above this one associated with the same
                //apptoken note that the window could be a floating window
                //that was created later or a window at the top of the list of
                //windows associated with this token.
                if (DEBUG_FOCUS_LIGHT || DEBUG_WINDOW_MOVEMENT || DEBUG_ADD_REMOVE) {
                    // Slogger::V(TAG, "not Base app: Adding window "
                    //         + win + " at " + (newIdx + 1) + " of " + N);
                }
                if (newIt == windows->End()) {
                    windows->Insert(windows->Begin(), win);
                    // No window from token found on win's display.
                    tokenWindowsIt = tokenWindowList->Begin();
                }
                else {
                    windows->Insert(++newIt, win);
                    tokenWindowsIt = ++IndexOfWinInWindowList(*newIt, token->mWindows);
                }
                mWindowsChanged = TRUE;
            }
        }
        return tokenWindowsIt;
    }

    // No windows from this token on this display
    if (localLOGV) {
        // Slog.v(TAG, "Figuring out where to add app window " + client.asBinder()
        //     + " (token=" + token + ")");
    }
    // Figure out where the window should go, based on the
    // order of applications.
    AutoPtr<WindowState> pos;

    AppTokenList::ReverseIterator tokenRit;
    AutoPtr<List<AutoPtr<Task> > > tasks = displayContent->GetTasks();
    List<AutoPtr<Task> >::ReverseIterator taskRit = tasks->RBegin();
    for (; taskRit != tasks->REnd(); ++taskRit) {
        AppTokenList tokens = (*taskRit)->mAppTokens;
        tokenRit = tokens.RBegin();
        for (; tokenRit != tokens.REnd(); ++tokenRit) {
            AutoPtr<AppWindowToken> t = *tokenRit;
            if (t == token) {
                ++tokenRit;
                if (tokenRit == tokens.REnd()) {
                    ++taskRit;
                    if (taskRit != tasks->REnd()) {
                        tokenRit = (*taskRit)->mAppTokens.RBegin();
                    }
                }
                break;
            }

            // We haven't reached the token yet; if this token
            // is not going to the bottom and has windows on this display, we can
            // use it as an anchor for when we do reach the token.
            tokenWindowList = GetTokenWindowsOnDisplay(t, displayContent);
            if (!t->mSendingToBottom && tokenWindowList->Begin() != tokenWindowList->End()) {
                pos = *(tokenWindowList->Begin());
            }
        }
        if (tokenRit != tokens.REnd()) {
            // early exit
            break;
        }
    }

    // We now know the index into the apps.  If we found
    // an app window above, that gives us the position; else
    // we need to look some more.
    if (pos != NULL) {
        // Move behind any windows attached to this one.
        AutoPtr<WindowToken> atoken;
        HashMap<AutoPtr<IBinder>, AutoPtr<WindowToken> >::Iterator tokenIt = mTokenMap.Find(IBinder::Probe(pos->mClient));
        if (tokenIt != mTokenMap.End()) {
            atoken = tokenIt->mSecond;
        }
        if (atoken != NULL) {
            tokenWindowList = GetTokenWindowsOnDisplay(atoken, displayContent);
            if (tokenWindowList->Begin() != tokenWindowList->End()) {
                AutoPtr<WindowState> bottom = *(tokenWindowList->Begin());
                if (bottom->mSubLayer < 0) {
                    pos = bottom;
                }
            }
        }
        PlaceWindowBefore(pos, win);
        return tokenWindowsIt;
    }

    // Continue looking down until we find the first
    // token that has windows on this display.
    for (; taskRit != tasks->REnd(); ++taskRit) {
        AppTokenList tokens = (*taskRit)->mAppTokens;
        for ( ; tokenRit != tokens.REnd(); ++tokenRit) {
            AutoPtr<AppWindowToken> t = *tokenRit;
            tokenWindowList = GetTokenWindowsOnDisplay(t, displayContent);
            if (tokenWindowList->Begin() != tokenWindowList->End()) {
                pos = *(tokenWindowList->End());
                break;
            }
        }
        if (tokenRit != tokens.REnd()) {
            // found
            break;
        }
    }

    if (pos != NULL) {
        // Move in front of any windows attached to this
        // one.
        AutoPtr<WindowToken> atoken;
        HashMap<AutoPtr<IBinder>, AutoPtr<WindowToken> >::Iterator tokenIt = mTokenMap.Find(IBinder::Probe(pos->mClient));
        if (tokenIt != mTokenMap.End()) {
            atoken = tokenIt->mSecond;
        }
        if (atoken != NULL) {
            if (atoken->mWindows.Begin() != atoken->mWindows.End()) {
                AutoPtr<WindowState> top = *(atoken->mWindows.End());
                if (top->mSubLayer >= 0) {
                    pos = top;
                }
            }
        }
        PlaceWindowAfter(pos, win);
        return tokenWindowsIt;
    }

    // Just search for the start of this layer.
    Int32 myLayer = win->mBaseLayer;
    WindowList::Iterator winIt = windows->Begin();
    for (; winIt != windows->End(); ++winIt) {
        AutoPtr<WindowState> w = *winIt;
        if (w->mBaseLayer > myLayer) {
            break;
        }
    }
    if (DEBUG_FOCUS_LIGHT || DEBUG_WINDOW_MOVEMENT || DEBUG_ADD_REMOVE) {
        // Slogger::V(TAG, "Based on layer: Adding window %p at " + win + " at " + i + " of " + N);
    }
    windows->Insert(winIt, win);
    mWindowsChanged = true;
    return tokenWindowsIt;
}

void CWindowManagerService::AddFreeWindowToListLocked(
    /* [in] */ WindowState* win)
{
    AutoPtr<WindowList> windows = win->GetWindowList();

    // Figure out where window should go, based on layer.
    Int32 myLayer = win->mBaseLayer;
    WindowList::ReverseIterator rit = windows->RBegin();
    for (; rit != windows->REnd(); ++rit) {
        if ((*rit)->mBaseLayer <= myLayer) {
            break;
        }
    }
    // if (DEBUG_FOCUS_LIGHT || DEBUG_WINDOW_MOVEMENT || DEBUG_ADD_REMOVE) Slog.v(TAG,
    //         "Free window: Adding window " + win + " at " + i + " of " + windows.size());
    windows->Insert(rit.GetBase(), win);
    mWindowsChanged = TRUE;
}

void CWindowManagerService::AddAttachedWindowToListLocked(
    /* [in] */ WindowState* win,
    /* [in] */ Boolean addToToken)
{
    AutoPtr<WindowToken> token = win->mToken;
    AutoPtr<DisplayContent> displayContent = win->GetDisplayContent();
    if (displayContent == NULL) {
        return;
    }
    AutoPtr<WindowState> attached = win->mAttachedWindow;

    AutoPtr<WindowList> tokenWindowList = GetTokenWindowsOnDisplay(token, displayContent);

    // Figure out this window's ordering relative to the window
    // it is attached to.
    Int32 sublayer = win->mSubLayer;
    Int32 largestSublayer = Elastos::Core::Math::INT32_MIN_VALUE;
    AutoPtr<WindowState> windowWithLargestSublayer;
    WindowList::Iterator it = tokenWindowList->Begin();
    for (; it != tokenWindowList->End(); ++it) {
        AutoPtr<WindowState> w = *it;
        Int32 wSublayer = w->mSubLayer;
        if (wSublayer >= largestSublayer) {
            largestSublayer = wSublayer;
            windowWithLargestSublayer = w;
        }
        if (sublayer < 0) {
            // For negative sublayers, we go below all windows
            // in the same sublayer.
            if (wSublayer >= sublayer) {
                if (addToToken) {
                    if (DEBUG_ADD_REMOVE) Slogger::V(TAG, "Adding %p to %p", win, token.Get());
                    token->mWindows.Insert(it, win);
                }
                PlaceWindowBefore(wSublayer >= 0 ? attached : w, win);
                break;
            }
        }
        else {
            // For positive sublayers, we go above all windows
            // in the same sublayer.
            if (wSublayer > sublayer) {
                if (addToToken) {
                    if (DEBUG_ADD_REMOVE) Slogger::V(TAG, "Adding %p to %p", win, token.Get());
                    token->mWindows.Insert(it, win);
                }
                PlaceWindowBefore(w, win);
                break;
            }
        }
    }
    if (it == tokenWindowList->End()) {
        if (addToToken) {
            if (DEBUG_ADD_REMOVE) Slogger::V(TAG, "Adding %p to %p", win, token.Get());
            token->mWindows.PushBack(win);
        }
        if (sublayer < 0) {
            PlaceWindowBefore(attached, win);
        }
        else {
            PlaceWindowAfter(largestSublayer >= 0 ? windowWithLargestSublayer : attached, win);
        }
    }
}

void CWindowManagerService::AddWindowToListInOrderLocked(
    /* [in] */ WindowState* win,
    /* [in] */ Boolean addToToken)
{
    if (DEBUG_FOCUS_LIGHT) Slogger::D(TAG, "addWindowToListInOrderLocked: win=%p Callers=", win/*, Debug.getCallers(4)*/);
    if (win->mAttachedWindow == NULL) {
        AutoPtr<WindowToken> token = win->mToken;
        List<AutoPtr<WindowState> >::Iterator tokenWindowsIt = token->mWindows.Begin();
        if (token->mAppWindowToken != NULL) {
            tokenWindowsIt = AddAppWindowToListLocked(win);
        }
        else {
            AddFreeWindowToListLocked(win);
        }
        if (addToToken) {
            if (DEBUG_ADD_REMOVE) Slogger::V(TAG, "Adding %p to %p", win, token.Get());
            token->mWindows.Insert(tokenWindowsIt, win);
        }
    }
    else {
        AddAttachedWindowToListLocked(win, addToToken);
    }

    if (win->mAppToken != NULL && addToToken) {
        win->mAppToken->mAllAppWindows.PushBack(win);
    }
}

Boolean CWindowManagerService::CanBeImeTarget(
    /* [in] */ WindowState* w)
{
    Int32 flags, type;
    w->mAttrs->GetFlags(&flags);
    Int32 fl = flags
            & (IWindowManagerLayoutParams::FLAG_NOT_FOCUSABLE | IWindowManagerLayoutParams::FLAG_ALT_FOCUSABLE_IM);
    if (fl == 0
            || fl == (IWindowManagerLayoutParams::FLAG_NOT_FOCUSABLE | IWindowManagerLayoutParams::FLAG_ALT_FOCUSABLE_IM)
            || (w->mAttrs->GetType(&type), type == IWindowManagerLayoutParams::TYPE_APPLICATION_STARTING)) {
        if (DEBUG_INPUT_METHOD) {
            Slogger::I(TAG, "isVisibleOrAdding %p: %d", w, w->IsVisibleOrAdding());
            if (!w->IsVisibleOrAdding()) {
                Slogger::I(TAG, "  mSurface=%p relayoutCalled=%d viewVis=%d policyVis=%d policyVisAfterAnim=%d attachHid=%d exiting=%d destroying=%d",
                        w->mWinAnimator->mSurfaceControl.Get(),
                        w->mRelayoutCalled, w->mViewVisibility,
                        w->mPolicyVisibility,
                        w->mPolicyVisibilityAfterAnim,
                        w->mAttachedHidden,
                        w->mExiting, w->mDestroying);
                if (w->mAppToken != NULL) {
                    Slogger::I(TAG, "  mAppToken.hiddenRequested=%d", w->mAppToken->mHiddenRequested);
                }
            }
        }
        return w->IsVisibleOrAdding();
    }
    return FALSE;
}

List< AutoPtr<WindowState> >::Iterator CWindowManagerService::FindDesiredInputMethodWindowItLocked(
    /* [in] */ Boolean willMove)
{
    // TODO(multidisplay): Needs some serious rethought when the target and IME are not on the
    // same display. Or even when the current IME/target are not on the same screen as the next
    // IME/target. For now only look for input windows on the main screen.
    AutoPtr<WindowList> windows = GetDefaultWindowListLocked();
    List< AutoPtr<WindowState> >::ReverseIterator rit = windows->RBegin();
    AutoPtr<WindowState> w;
    for (; rit != windows->REnd(); ++rit) {
        AutoPtr<WindowState> win = *rit;

        // if (DEBUG_INPUT_METHOD && willMove) Slog.i(TAG, "Checking window @" + i
        //         + " " + win + " fl=0x" + Integer.toHexString(w.mAttrs.flags));
        if (CanBeImeTarget(win)) {
            w = win;
            // Slogger::I(TAG, "Putting input method here!");

            // Yet more tricksyness!  If this window is a "starting"
            // window, we do actually want to be on top of it, but
            // it is not -really- where input will go.  So if the caller
            // is not actually looking to move the IME, look down below
            // for a real window to target...
            Int32 type;
            if (!willMove
                    && (w->mAttrs->GetType(&type), type == IWindowManagerLayoutParams::TYPE_APPLICATION_STARTING)
                    && rit != --windows->REnd()) {
                List< AutoPtr<WindowState> >::ReverseIterator trit = rit;
                AutoPtr<WindowState> wb = *(++trit);
                if (wb->mAppToken == w->mAppToken && CanBeImeTarget(wb)) {
                    ++rit;
                    w = wb;
                }
            }
            break;
        }
    }

    // Now w is either mWindows[0] or an IME (or null if mWindows is empty).

    if (DEBUG_INPUT_METHOD && willMove) Slogger::V(TAG, "Proposed new IME target: %p", w.Get());

    // Now, a special case -- if the last target's window is in the
    // process of exiting, and is above the new target, keep on the
    // last target to avoid flicker.  Consider for example a Dialog with
    // the IME shown: when the Dialog is dismissed, we want to keep
    // the IME above it until it is completely gone so it doesn't drop
    // behind the dialog or its full-screen scrim.
    AutoPtr<WindowState> curTarget = mInputMethodTarget;
    Boolean isDisplayed;
    if (curTarget != NULL
            && (curTarget->IsDisplayedLw(&isDisplayed), isDisplayed)
            && curTarget->IsClosing()
            && (w == NULL || curTarget->mWinAnimator->mAnimLayer > w->mWinAnimator->mAnimLayer)) {
        if (DEBUG_INPUT_METHOD) Slogger::V(TAG, "Current target higher, not changing");
        List< AutoPtr<WindowState> >::Iterator result = Find(*windows, curTarget);
        return ++result;
    }

    if (DEBUG_INPUT_METHOD) Slogger::V(TAG, "Desired input method target=%p willMove=%d",
            w.Get(), willMove);

    if (willMove && w != NULL) {
        AutoPtr<AppWindowToken> token = curTarget == NULL ? NULL : curTarget->mAppToken;
        if (token != NULL) {

            // Now some fun for dealing with window animations that
            // modify the Z order.  We need to look at all windows below
            // the current target that are in this app, finding the highest
            // visible one in layering.
            AutoPtr<WindowState> highestTarget;
            List< AutoPtr<WindowState> >::Iterator highestPosIt;
            if (token->mAppAnimator->mAnimating || token->mAppAnimator->mAnimation != NULL) {
                AutoPtr<WindowList> curWindows = curTarget->GetWindowList();
                List< AutoPtr<WindowState> >::ReverseIterator posRIt(Find(*curWindows, curTarget));
                while (posRIt != curWindows->REnd()) {
                    AutoPtr<WindowState> win = *posRIt;
                    if (win->mAppToken != token) {
                        break;
                    }
                    if (!win->mRemoved) {
                        if (highestTarget == NULL || win->mWinAnimator->mAnimLayer >
                                highestTarget->mWinAnimator->mAnimLayer) {
                            highestTarget = win;
                            highestPosIt = --(posRIt.GetBase());
                        }
                    }
                    ++posRIt;
                }
            }

            if (highestTarget != NULL) {
                if (DEBUG_INPUT_METHOD) Slogger::V(TAG, "%p %p animating=%d,  layer=%d new layer=%d"
                        , mAppTransition.Get(), highestTarget.Get()
                        , highestTarget->mWinAnimator->IsAnimating()
                        , highestTarget->mWinAnimator->mAnimLayer
                        , w->mWinAnimator->mAnimLayer);

                if (mAppTransition->IsTransitionSet()) {
                    // If we are currently setting up for an animation,
                    // hold everything until we can find out what will happen.
                    mInputMethodTargetWaitingAnim = TRUE;
                    mInputMethodTarget = highestTarget;
                    return ++highestPosIt;
                }
                else if (highestTarget->mWinAnimator->IsAnimating() &&
                        highestTarget->mWinAnimator->mAnimLayer > w->mWinAnimator->mAnimLayer) {
                    // If the window we are currently targeting is involved
                    // with an animation, and it is on top of the next target
                    // we will be over, then hold off on moving until
                    // that is done.
                    mInputMethodTargetWaitingAnim = TRUE;
                    mInputMethodTarget = highestTarget;
                    return ++highestPosIt;
                }
            }
        }
    }

    // Slog.i(TAG, "Placing input method @" + (i+1));
    if (w != NULL) {
        if (willMove) {
            // if (DEBUG_INPUT_METHOD) Slog.w(TAG, "Moving IM target from " + curTarget + " to "
            //         + w + (HIDE_STACK_CRAWLS ? "" : " Callers=" + Debug.getCallers(4)));
            mInputMethodTarget = w;
            mInputMethodTargetWaitingAnim = FALSE;
            if (w->mAppToken != NULL) {
                SetInputMethodAnimLayerAdjustment(w->mAppToken->mAppAnimator->mAnimLayerAdjustment);
            }
            else {
                SetInputMethodAnimLayerAdjustment(0);
            }
        }
        return rit.GetBase();
    }
    if (willMove) {
        // if (DEBUG_INPUT_METHOD) Slog.w(TAG, "Moving IM target from " + curTarget + " to null."
        //             + (HIDE_STACK_CRAWLS ? "" : " Callers=" + Debug.getCallers(4)));
        mInputMethodTarget = NULL;
        SetInputMethodAnimLayerAdjustment(0);
    }

    return windows->End();
}

void CWindowManagerService::AddInputMethodWindowToListLocked(
    /* [in] */ WindowState* win)
{
    AutoPtr<WindowList> windows = GetDefaultWindowListLocked();
    List< AutoPtr<WindowState> >::Iterator posIt = FindDesiredInputMethodWindowItLocked(TRUE);
    if (posIt != windows->End()) {
        win->mTargetAppToken = mInputMethodTarget->mAppToken;
        // if (DEBUG_WINDOW_MOVEMENT) Slog.v(
        //         TAG, "Adding input method window " + win + " at " + pos);
        windows->Insert(posIt, win);
        mWindowsChanged = TRUE;
        MoveInputMethodDialogsLocked(++posIt);
        return;
    }
    win->mTargetAppToken = NULL;
    AddWindowToListInOrderLocked(win, TRUE);
    MoveInputMethodDialogsLocked(posIt);
}

void CWindowManagerService::SetInputMethodAnimLayerAdjustment(
    /* [in] */ Int32 adj)
{
    if (DEBUG_LAYERS) Slogger::V(TAG, "Setting im layer adj to %d", adj);
    mInputMethodAnimLayerAdjustment = adj;
    AutoPtr<WindowState> imw = mInputMethodWindow;
    List<AutoPtr<WindowState> >::ReverseIterator rit;
    if (imw != NULL) {
        imw->mWinAnimator->mAnimLayer = imw->mLayer + adj;
        if (DEBUG_LAYERS) Slogger::V(TAG, "IM win %p anim layer: %d", imw.Get(),
                imw->mWinAnimator->mAnimLayer);
        for (rit = imw->mChildWindows.RBegin(); rit != imw->mChildWindows.REnd(); ++rit) {
            AutoPtr<WindowState> cw = *rit;
            cw->mWinAnimator->mAnimLayer = cw->mLayer + adj;
            if (DEBUG_LAYERS) Slogger::V(TAG, "IM win %p anim layer: %d", cw.Get(),
                    cw->mWinAnimator->mAnimLayer);
        }
    }
    for (rit = mInputMethodDialogs.RBegin(); rit != mInputMethodDialogs.REnd(); ++rit) {
        imw = *rit;
        imw->mWinAnimator->mAnimLayer = imw->mLayer + adj;
        if (DEBUG_LAYERS) Slogger::V(TAG, "IM win %p anim layer: %d", imw.Get(),
                imw->mWinAnimator->mAnimLayer);
    }
}

List< AutoPtr<WindowState> >::Iterator CWindowManagerService::TmpRemoveWindowLocked(
    /* [in] */ List< AutoPtr<WindowState> >::Iterator interestingPosIt,
    /* [in] */ WindowState* win)
{
    AutoPtr<WindowState> aWin = win;
    AutoPtr<WindowList> windows = win->GetWindowList();
    List<AutoPtr<WindowState> >::Iterator wposIt = Find(*windows, aWin);
    if (wposIt != windows->End()) {
        if (DEBUG_WINDOW_MOVEMENT) Slogger::V(TAG, "Temp removing : %p", win);
        windows->Erase(wposIt);
        mWindowsChanged = true;
        List<AutoPtr<WindowState> >::ReverseIterator rit = win->mChildWindows.RBegin();
        for (; rit != win->mChildWindows.REnd(); ++rit) {
            AutoPtr<WindowState> cw = *rit;
            List<AutoPtr<WindowState> >::Iterator cposIt = Find(*windows, cw);
            if (cposIt != windows->End()) {
                if (DEBUG_WINDOW_MOVEMENT) Slogger::V(TAG, "Temp removing child : %p", cw.Get());
                windows->Erase(cposIt);
            }
        }
    }
    return interestingPosIt;
}

void CWindowManagerService::ReAddWindowToListInOrderLocked(
    /* [in] */ WindowState* win)
{
    AddWindowToListInOrderLocked(win, FALSE);
    // This is a hack to get all of the child windows added as well
    // at the right position.  Child windows should be rare and
    // this case should be rare, so it shouldn't be that big a deal.
    AutoPtr<WindowList> windows = win->GetWindowList();
    List<AutoPtr<WindowState> >::Iterator it = Find(*windows, win);
    if (it != windows->End()) {
        if (DEBUG_WINDOW_MOVEMENT) Slogger::V(TAG, "ReAdd removing from : %p", win);
        windows->Erase(it);
        mWindowsChanged = TRUE;
        ReAddWindowLocked(it, win);
    }
}

void CWindowManagerService::LogWindowList(
    /* [in] */ List< AutoPtr<WindowState> >& windows,
    /* [in] */ const String& prefix)
{
    List<AutoPtr<WindowState> >::ReverseIterator rit;
    Int32 i = 0;
    for (rit = windows.RBegin(); rit != windows.REnd(); ++rit, ++i) {
       Slogger::V(TAG, "%s#: %d, %p", prefix.string(), i, (*rit).Get());
       PFL_EX("%s#: %d, %p", prefix.string(), i, (*rit).Get());
    }
}

void CWindowManagerService::MoveInputMethodDialogsLocked(
    /* [in] */ List< AutoPtr<WindowState> >::Iterator posIt)
{
    List< AutoPtr<WindowState> >& dialogs = mInputMethodDialogs;

    // TODO(multidisplay): IMEs are only supported on the default display.
    AutoPtr<WindowList> windows = GetDefaultWindowListLocked();
    WindowList::Iterator it = dialogs.Begin();
    // if (DEBUG_INPUT_METHOD) Slog.v(TAG, "Removing " + N + " dialogs w/pos=" + pos);
    for (; it != dialogs.End(); ++it) {
        posIt = TmpRemoveWindowLocked(posIt, *it);
    }
    // if (DEBUG_INPUT_METHOD) {
    //     Slog.v(TAG, "Window list w/pos=" + pos);
    //     logWindowList("  ");
    // }

    if (posIt != windows->End()) {
        AutoPtr<AppWindowToken> targetAppToken = mInputMethodTarget->mAppToken;
        // Skip windows owned by the input method.
        if (mInputMethodWindow != NULL) {
            while (posIt != windows->End()) {
                AutoPtr<WindowState> wp = *posIt;
                if (wp == mInputMethodWindow || wp->mAttachedWindow == mInputMethodWindow) {
                    posIt++;
                    continue;
                }
                break;
            }
        }
        // if (DEBUG_INPUT_METHOD) Slog.v(TAG, "Adding " + N + " dialogs at pos=" + pos);
        for (it = dialogs.Begin(); it != dialogs.End(); ++it) {
            AutoPtr<WindowState> win = *it;
            win->mTargetAppToken = targetAppToken;
            posIt = ReAddWindowLocked(posIt, win);
        }
        // if (DEBUG_INPUT_METHOD) {
        //     Slog.v(TAG, "Final window list:");
        //     logWindowList(windows, "  ");
        // }
        return;
    }
    for (it = dialogs.Begin(); it != dialogs.End(); ++it) {
        AutoPtr<WindowState> win = *it;
        win->mTargetAppToken = NULL;
        ReAddWindowToListInOrderLocked(win);
//        if (DEBUG_INPUT_METHOD) {
//            Slog.v(TAG, "No IM target, final list:");
//            logWindowList("  ");
//        }
    }
}

Boolean CWindowManagerService::MoveInputMethodWindowsIfNeededLocked(
    /* [in] */ Boolean needAssignLayers)
{
    AutoPtr<WindowState> imWin = mInputMethodWindow;
    if (imWin == NULL && mInputMethodDialogs.Begin() == mInputMethodDialogs.End()) {
        return FALSE;
    }

    // TODO(multidisplay): IMEs are only supported on the default display.
    AutoPtr<WindowList> windows = GetDefaultWindowListLocked();
    WindowList::Iterator imPosIt = FindDesiredInputMethodWindowItLocked(TRUE);
    if (imPosIt != windows->End()) {
        // In this case, the input method windows are to be placed
        // immediately above the window they are targeting.

        // First check to see if the input method windows are already
        // located here, and contiguous.
        AutoPtr<WindowState> firstImWin = (imPosIt != windows->End()) ? *imPosIt : NULL;

        // Figure out the actual input method window that should be
        // at the bottom of their stack.
        AutoPtr<WindowState> baseImWin = imWin != NULL
                ? imWin : *(mInputMethodDialogs.Begin());
        if (baseImWin->mChildWindows.Begin() != baseImWin->mChildWindows.End()) {
            AutoPtr<WindowState> cw = *baseImWin->mChildWindows.Begin();
            if (cw->mSubLayer < 0) baseImWin = cw;
        }

        if (firstImWin == baseImWin) {
            // The windows haven't moved...  but are they still contiguous?
            // First find the top IM window.
            List< AutoPtr<WindowState> >::Iterator posIt = imPosIt;
            if (++posIt != windows->End()) {
                for (; posIt != windows->End(); ++posIt) {
                    if (!(*posIt)->mIsImWindow) {
                        break;
                    }
                }

                if (posIt != windows->End()) ++posIt;

                // Now there should be no more input method windows above.
                for (; posIt != windows->End(); ++posIt) {
                    if ((*posIt)->mIsImWindow) {
                        break;
                    }
                }
            }

            if (posIt == windows->End()) {
                // Z order is good.
                // The IM target window may be changed, so update the mTargetAppToken.
                if (imWin != NULL) {
                    imWin->mTargetAppToken = mInputMethodTarget->mAppToken;
                }
                return FALSE;
            }
        }

        if (imWin != NULL) {
            // if (DEBUG_INPUT_METHOD) {
            //     Slog.v(TAG, "Moving IM from " + imPos);
            //     logWindowList(windows, "  ");
            // }
            imPosIt = TmpRemoveWindowLocked(imPosIt, imWin);
            // if (DEBUG_INPUT_METHOD) {
            //     Slog.v(TAG, "List after removing with new pos " + imPos + ":");
            //     logWindowList(windows, "  ");
            // }
            imWin->mTargetAppToken = mInputMethodTarget->mAppToken;
            ReAddWindowLocked(imPosIt, imWin);
            // if (DEBUG_INPUT_METHOD) {
            //     Slog.v(TAG, "List after moving IM to " + imPos + ":");
            //     logWindowList(windows, "  ");
            // }
            if (mInputMethodDialogs.Begin() != mInputMethodDialogs.End()) {
                List< AutoPtr<WindowState> >::Iterator temp = imPosIt;
                MoveInputMethodDialogsLocked(++temp);
            }
        }
        else {
            MoveInputMethodDialogsLocked(imPosIt);
        }
    }
    else {
        // In this case, the input method windows go in a fixed layer,
        // because they aren't currently associated with a focus window.
        if (imWin != NULL) {
            // if (DEBUG_INPUT_METHOD) Slog.v(TAG, "Moving IM from " + imPos);
            TmpRemoveWindowLocked(windows->Begin(), imWin);
            imWin->mTargetAppToken = NULL;
            ReAddWindowToListInOrderLocked(imWin);
            // if (DEBUG_INPUT_METHOD) {
            //     Slog.v(TAG, "List with no IM target:");
            //     logWindowList("  ");
            // }
            if (mInputMethodDialogs.Begin() != mInputMethodDialogs.End()) {
                MoveInputMethodDialogsLocked(windows->End());
            }
        }
        else {
            MoveInputMethodDialogsLocked(windows->End());
        }

    }

    if (needAssignLayers) {
        AssignLayersLocked(windows);
    }

    return TRUE;
}

Boolean CWindowManagerService::IsWallpaperVisible(
    /* [in] */ WindowState* wallpaperTarget)
{
    // if (DEBUG_WALLPAPER) Slog.v(TAG, "Wallpaper vis: target " + wallpaperTarget + ", obscured="
    //         + (wallpaperTarget != null ? Boolean.toString(wallpaperTarget.mObscured) : "??")
    //         + " anim=" + ((wallpaperTarget != null && wallpaperTarget.mAppToken != null)
    //                 ? wallpaperTarget.mAppToken.mAppAnimator.animation : null)
    //         + " upper=" + mUpperWallpaperTarget
    //         + " lower=" + mLowerWallpaperTarget);
    return (wallpaperTarget != NULL
                    && (!wallpaperTarget->mObscured || (wallpaperTarget->mAppToken != NULL
                            && wallpaperTarget->mAppToken->mAppAnimator->mAnimation != NULL)))
            || mUpperWallpaperTarget != NULL
            || mLowerWallpaperTarget != NULL;
}

Int32 CWindowManagerService::AdjustWallpaperWindowsLocked()
{
    mInnerFields->mWallpaperMayChange = FALSE;
    Boolean targetChanged = FALSE;

    // TODO(multidisplay): Wallpapers on main screen only.
    AutoPtr<IDisplayInfo> displayInfo
            = GetDefaultDisplayContentLocked()->GetDisplayInfo();
    Int32 dw, dh;
    displayInfo->GetLogicalWidth(&dw);
    displayInfo->GetLogicalHeight(&dh);

    // First find top-most window that has asked to be on top of the
    // wallpaper; all wallpapers go behind it.
    AutoPtr<WindowList> windows = GetDefaultWindowListLocked();
    AutoPtr<WindowState> w;
    AutoPtr<WindowState> foundW;
    WindowList::Iterator foundIt = windows->Begin();
    AutoPtr<WindowState> topCurW;
    WindowList::Iterator topCurIt = windows->Begin();
    WindowList::Iterator windowDetachedIt = windows->End();
    WindowList::ReverseIterator rit;
    for (rit = windows->RBegin(); rit != windows->REnd(); ++rit) {
        w = *rit;
        Int32 wType;
        w->mAttrs->GetType(&wType);
        if ((wType == IWindowManagerLayoutParams::TYPE_WALLPAPER)) {
            if (topCurW == NULL) {
                topCurW = w;
                topCurIt = --rit.GetBase();
            }
            continue;
        }
        topCurW = NULL;
        if (w != mAnimator->mWindowDetachedWallpaper && w->mAppToken != NULL) {
            // If this window's app token is hidden and not animating,
            // it is of no interest to us.
            if (w->mAppToken->mHidden && w->mAppToken->mAppAnimator->mAnimation == NULL) {
                if (DEBUG_WALLPAPER) Slogger::V(TAG,
                        "Skipping hidden and not animating token: %p", w.Get());
                continue;
            }
        }
        if (DEBUG_WALLPAPER) Slogger::V(TAG, "Win # %p: isOnScreen=%d mDrawState=%d"
                ,/* i */ w.Get(), w->IsOnScreen(), w->mWinAnimator->mDrawState);

        // If the app is executing an animation because the keyguard is going away, keep the
        // wallpaper during the animation so it doesn't flicker out.
        Int32 flags;
        Boolean hasWallpaper = (w->mAttrs->GetFlags(&flags), (flags & IWindowManagerLayoutParams::FLAG_SHOW_WALLPAPER) != 0)
                || (w->mAppToken != NULL && w->mWinAnimator->mKeyguardGoingAwayAnimation);
        if (hasWallpaper && w->IsOnScreen()
                && (mWallpaperTarget == w || w->IsDrawFinishedLw())) {
            if (DEBUG_WALLPAPER) Slogger::V(TAG, "Found wallpaper target: # =%p",/* i */w.Get());
            foundW = w;
            foundIt = --rit.GetBase();
            if (w == mWallpaperTarget && w->mWinAnimator->IsAnimating()) {
                // The current wallpaper target is animating, so we'll
                // look behind it for another possible target and figure
                // out what is going on below.
                if (DEBUG_WALLPAPER) Slogger::V(TAG, "Win %p: token animating, looking behind.", w.Get());
                continue;
            }
            break;
        }
        else if (w == mAnimator->mWindowDetachedWallpaper) {
            windowDetachedIt = --rit.GetBase();
        }
    }

    if (foundW == NULL && windowDetachedIt != windows->End()) {
        // if (DEBUG_WALLPAPER_LIGHT) Slog.v(TAG,
        //         "Found animating detached wallpaper activity: #" + i + "=" + w);
        foundW = w;
        foundIt = windowDetachedIt;
    }

    if (mWallpaperTarget != foundW
            && (mLowerWallpaperTarget == NULL || mLowerWallpaperTarget != foundW)) {
        if (DEBUG_WALLPAPER_LIGHT) {
            Slogger::V(TAG, "New wallpaper target: %p oldTarget: %p", foundW.Get(),
                    mWallpaperTarget.Get());
        }

        mLowerWallpaperTarget = NULL;
        mUpperWallpaperTarget = NULL;

        AutoPtr<WindowState> oldW = mWallpaperTarget;
        mWallpaperTarget = foundW;
        targetChanged = TRUE;

        // Now what is happening...  if the current and new targets are
        // animating, then we are in our super special mode!
        if (foundW != NULL && oldW != NULL) {
            Boolean oldAnim;
            oldW->IsAnimatingLw(&oldAnim);
            Boolean foundAnim;
            foundW->IsAnimatingLw(&foundAnim);
            if (DEBUG_WALLPAPER_LIGHT) {
                Slogger::V(TAG, "New animation: %d old animation: %d", foundAnim,
                        oldAnim);
            }
            if (foundAnim && oldAnim) {
                WindowList::Iterator oldIt = Find(*windows, oldW);
                // if (DEBUG_WALLPAPER_LIGHT) {
                //     Slog.v(TAG, "New i: " + foundI + " old i: " + oldI);
                // }
                if (oldIt != windows->End()) {
                    // if (DEBUG_WALLPAPER_LIGHT) {
                    //     Slog.v(TAG, "Animating wallpapers: old#" + oldI
                    //             + "=" + oldW + "; new#" + foundI
                    //             + "=" + foundW);
                    // }

                    // Set the new target correctly.
                    if (foundW->mAppToken != NULL && foundW->mAppToken->mHiddenRequested) {
                        if (DEBUG_WALLPAPER) {
                            Slogger::V(TAG, "Old wallpaper still the target.");
                        }
                        mWallpaperTarget = oldW;
                        foundW = oldW;
                        foundIt = oldIt;
                    }
                    // Now set the upper and lower wallpaper targets
                    // correctly, and make sure that we are positioning
                    // the wallpaper below the lower.
                    else {
                        Boolean isUpper = TRUE;
                        WindowList::Iterator it = windows->Begin();
                        for (; it != windows->End(); ++it) {
                            if (it == foundIt) {
                                isUpper = FALSE;
                                break;
                            }
                            if (it == oldIt) {
                                isUpper = TRUE;
                                break;
                            }
                        }
                        if (isUpper) {
                            // The new target is on top of the old one.
                            if (DEBUG_WALLPAPER_LIGHT) {
                                Slogger::V(TAG, "Found target above old target.");
                            }
                            mUpperWallpaperTarget = foundW;
                            mLowerWallpaperTarget = oldW;
                            foundW = oldW;
                            foundIt = oldIt;
                        }
                        else {
                            // The new target is below the old one.
                            if (DEBUG_WALLPAPER_LIGHT) {
                                Slogger::V(TAG, "Found target below old target.");
                            }
                            mUpperWallpaperTarget = oldW;
                            mLowerWallpaperTarget = foundW;
                        }
                    }
                }
            }
        }
    }
    else if (mLowerWallpaperTarget != NULL) {
        // Is it time to stop animating?
        Boolean isAnimatingLower, isAnimatingUpper;
        if ((mLowerWallpaperTarget->IsAnimatingLw(&isAnimatingLower), !isAnimatingLower)
                || (mUpperWallpaperTarget->IsAnimatingLw(&isAnimatingUpper), !isAnimatingUpper)) {
            if (DEBUG_WALLPAPER_LIGHT) {
                Slogger::V(TAG, "No longer animating wallpaper targets!");
            }
            mLowerWallpaperTarget = NULL;
            mUpperWallpaperTarget = NULL;
            mWallpaperTarget = foundW;
            targetChanged = TRUE;
        }
    }

    Boolean visible = foundW != NULL;
    if (visible) {
        // The window is visible to the compositor...  but is it visible
        // to the user?  That is what the wallpaper cares about.
        visible = IsWallpaperVisible(foundW);
        if (DEBUG_WALLPAPER) Slogger::V(TAG, "Wallpaper visibility: %d", visible);

        // If the wallpaper target is animating, we may need to copy
        // its layer adjustment.  Only do this if we are not transfering
        // between two wallpaper targets.
        mWallpaperAnimLayerAdjustment =
                (mLowerWallpaperTarget == NULL && foundW->mAppToken != NULL)
                ? foundW->mAppToken->mAppAnimator->mAnimLayerAdjustment : 0;

        Int32 wallpaperLayer;
        mPolicy->GetMaxWallpaperLayer(&wallpaperLayer);
        Int32 maxLayer = wallpaperLayer * TYPE_LAYER_MULTIPLIER + TYPE_LAYER_OFFSET;

        // Now w is the window we are supposed to be behind...  but we
        // need to be sure to also be behind any of its attached windows,
        // AND any starting window associated with it, AND below the
        // maximum layer the policy allows for wallpapers.
        while (foundIt != windows->Begin()) {
            AutoPtr<WindowState> wb = *(--WindowList::Iterator(foundIt));
            Int32 wbType;
            wb->mAttrs->GetType(&wbType);
            if (wb->mBaseLayer < maxLayer &&
                    wb->mAttachedWindow != foundW &&
                    (foundW->mAttachedWindow == NULL ||
                            wb->mAttachedWindow != foundW->mAttachedWindow) &&
                    (wbType != IWindowManagerLayoutParams::TYPE_APPLICATION_STARTING ||
                            foundW->mToken == NULL || wb->mToken != foundW->mToken)) {
                // This window is not related to the previous one in any
                // interesting way, so stop here.
                break;
            }
            foundW = wb;
            foundIt--;
        }
    }
    else {
        if (DEBUG_WALLPAPER) Slogger::V(TAG, "No wallpaper target");
    }

    if (foundW == NULL && topCurW != NULL) {
        // There is no wallpaper target, so it goes at the bottom.
        // We will assume it is the same place as last time, if known.
        foundW = topCurW;
        foundIt = ++WindowList::Iterator(topCurIt);
    }
    else {
        // Okay i is the position immediately above the wallpaper.  Look at
        // what is below it for later.
        foundW = foundIt != windows->Begin()
                ? *(--WindowList::Iterator(foundIt)) : NULL;
    }

    if (visible) {
        if (mWallpaperTarget->mWallpaperX >= 0) {
            mLastWallpaperX = mWallpaperTarget->mWallpaperX;
            mLastWallpaperXStep = mWallpaperTarget->mWallpaperXStep;
        }
        if (mWallpaperTarget->mWallpaperY >= 0) {
            mLastWallpaperY = mWallpaperTarget->mWallpaperY;
            mLastWallpaperYStep = mWallpaperTarget->mWallpaperYStep;
        }
        if (mWallpaperTarget->mWallpaperDisplayOffsetX != Elastos::Core::Math::INT32_MIN_VALUE) {
            mLastWallpaperDisplayOffsetX = mWallpaperTarget->mWallpaperDisplayOffsetX;
        }
        if (mWallpaperTarget->mWallpaperDisplayOffsetY != Elastos::Core::Math::INT32_MIN_VALUE) {
            mLastWallpaperDisplayOffsetY = mWallpaperTarget->mWallpaperDisplayOffsetY;
        }
    }

    // Start stepping backwards from here, ensuring that our wallpaper windows
    // are correctly placed.
    Int32 changed = 0;
    List< AutoPtr<WindowToken> >::ReverseIterator wtRit;
    for (wtRit = mWallpaperTokens.RBegin(); wtRit != mWallpaperTokens.REnd(); ++wtRit) {
        AutoPtr<WindowToken> token = *wtRit;
        if (token->mHidden == visible) {
            if (DEBUG_WALLPAPER_LIGHT) Slogger::D(TAG,
                    "Wallpaper token %p hidden=%d", token.Get(), !visible);
            changed |= ADJUST_WALLPAPER_VISIBILITY_CHANGED;
            token->mHidden = !visible;
            // Need to do a layout to ensure the wallpaper now has the
            // correct size.
            GetDefaultDisplayContentLocked()->mLayoutNeeded = TRUE;
        }

        for (rit = token->mWindows.RBegin(); rit != token->mWindows.REnd(); ++rit) {
            AutoPtr<WindowState> wallpaper = *rit;

            if (visible) {
                UpdateWallpaperOffsetLocked(wallpaper, dw, dh, FALSE);
            }

            // First, make sure the client has the current visibility
            // state.
            DispatchWallpaperVisibility(wallpaper, visible);

            wallpaper->mWinAnimator->mAnimLayer = wallpaper->mLayer
                    + mWallpaperAnimLayerAdjustment;
            if (DEBUG_LAYERS || DEBUG_WALLPAPER_LIGHT) Slogger::V(TAG, "adjustWallpaper win %p anim layer: %d",
                    wallpaper.Get(), wallpaper->mWinAnimator->mAnimLayer);

            // First, if this window is at the current index, then all
            // is well.
            if (wallpaper == foundW) {
                foundIt--;
                foundW = foundIt != windows->Begin()
                        ? *(--List< AutoPtr<WindowState> >::Iterator(foundIt)) : NULL;
                continue;
            }

            // The window didn't match...  the current wallpaper window,
            // wherever it is, is in the wrong place, so make sure it is
            // not in the list.
            List< AutoPtr<WindowState> >::Iterator oldIt = Find(*windows, wallpaper);
            if (oldIt != windows->End()) {
                // if (DEBUG_WINDOW_MOVEMENT) Slogger::V(TAG, "Wallpaper removing at %d: %p",
                //         oldIndex, wallpaper.Get());
                windows->Erase(oldIt);
                mWindowsChanged = TRUE;
            }

            // Now stick it in. For apps over wallpaper keep the wallpaper at the bottommost
            // layer. For keyguard over wallpaper put the wallpaper under the keyguard.
            List< AutoPtr<WindowState> >::Iterator insertionIt = windows->Begin();
            if (visible && foundW != NULL) {
                Int32 type;
                foundW->mAttrs->GetType(&type);
                Int32 privateFlags;
                foundW->mAttrs->GetPrivateFlags(&privateFlags);
                if ((privateFlags & IWindowManagerLayoutParams::PRIVATE_FLAG_KEYGUARD) != 0
                        || type == IWindowManagerLayoutParams::TYPE_KEYGUARD_SCRIM) {
                    insertionIt = Find(*windows, foundW);
                }
            }
            if (DEBUG_WALLPAPER_LIGHT || DEBUG_WINDOW_MOVEMENT || DEBUG_ADD_REMOVE) {
                Slogger::V(TAG, "Moving wallpaper %p from %p to %p", wallpaper.Get(), (*oldIt).Get(), (*insertionIt).Get());
            }

            windows->Insert(insertionIt, wallpaper);
            mWindowsChanged = TRUE;
            changed |= ADJUST_WALLPAPER_LAYERS_CHANGED;
        }
    }

    /*
    final TaskStack targetStack =
            mWallpaperTarget == null ? null : mWallpaperTarget.getStack();
    if ((changed & ADJUST_WALLPAPER_LAYERS_CHANGED) != 0 &&
            targetStack != null && !targetStack.isHomeStack()) {
        // If the wallpaper target is not on the home stack then make sure that all windows
        // from other non-home stacks are above the wallpaper.
        for (i = foundI - 1; i >= 0; --i) {
            WindowState win = windows.get(i);
            if (!win.isVisibleLw()) {
                continue;
            }
            final TaskStack winStack = win.getStack();
            if (winStack != null && !winStack.isHomeStack() && winStack != targetStack) {
                windows.remove(i);
                windows.add(foundI + 1, win);
            }
        }
    }
    */

    if (targetChanged && DEBUG_WALLPAPER_LIGHT) {
        Slogger::D(TAG, "New wallpaper: target=%p lower=%p upper=%p", mWallpaperTarget.Get(),
                mLowerWallpaperTarget.Get(), mUpperWallpaperTarget.Get());
    }

    return changed;
}

void CWindowManagerService::SetWallpaperAnimLayerAdjustmentLocked(
    /* [in] */ Int32 adj)
{
    if (DEBUG_LAYERS || DEBUG_WALLPAPER) Slogger::V(TAG, "Setting wallpaper layer adj to %d", adj);
    mWallpaperAnimLayerAdjustment = adj;
    List<AutoPtr<WindowToken> >::ReverseIterator rit;
    for (rit = mWallpaperTokens.RBegin(); rit != mWallpaperTokens.REnd(); ++rit) {
        AutoPtr<WindowToken> token = *rit;
        WindowList::ReverseIterator writ = token->mWindows.RBegin();
        for (; writ != token->mWindows.REnd(); ++writ) {
            AutoPtr<WindowState> wallpaper = *writ;
            wallpaper->mWinAnimator->mAnimLayer = wallpaper->mLayer + adj;
            if (DEBUG_LAYERS || DEBUG_WALLPAPER) Slogger::V(TAG, "setWallpaper win %p anim layer: %d",
                    wallpaper.Get(), wallpaper->mWinAnimator->mAnimLayer);
        }
    }
}

Boolean CWindowManagerService::UpdateWallpaperOffsetLocked(
    /* [in] */ WindowState* wallpaperWin,
    /* [in] */ Int32 dw,
    /* [in] */ Int32 dh,
    /* [in] */ Boolean sync)
{
    Boolean changed = FALSE;
    Boolean rawChanged = FALSE;
    Float wpx = mLastWallpaperX >= 0 ? mLastWallpaperX : 0.5;
    Float wpxs = mLastWallpaperXStep >= 0 ? mLastWallpaperXStep : -1.0;
    Int32 wallpaperWinRight, wallpaperWinLeft;
    wallpaperWin->mFrame->GetRight(&wallpaperWinRight);
    wallpaperWin->mFrame->GetLeft(&wallpaperWinLeft);
    Int32 availw = wallpaperWinRight - wallpaperWinLeft - dw;
    Int32 offset = availw > 0 ? - (Int32)(availw * wpx + 0.5) : 0;
    if (mLastWallpaperDisplayOffsetX != Elastos::Core::Math::INT32_MIN_VALUE) {
        offset += mLastWallpaperDisplayOffsetX;
    }
    changed = wallpaperWin->mXOffset != offset;
    if (changed) {
        if (DEBUG_WALLPAPER) Slogger::V(TAG, "Update wallpaper %p x: %d",
                wallpaperWin, offset);
        wallpaperWin->mXOffset = offset;
    }
    if (wallpaperWin->mWallpaperX != wpx || wallpaperWin->mWallpaperXStep != wpxs) {
        wallpaperWin->mWallpaperX = wpx;
        wallpaperWin->mWallpaperXStep = wpxs;
        rawChanged = TRUE;
    }

    Float wpy = mLastWallpaperY >= 0 ? mLastWallpaperY : 0.5;
    Float wpys = mLastWallpaperYStep >= 0 ? mLastWallpaperYStep : -1.0;
    Int32 wallpaperWinBottom, wallpaperWinTop;
    wallpaperWin->mFrame->GetBottom(&wallpaperWinBottom);
    wallpaperWin->mFrame->GetTop(&wallpaperWinTop);
    Int32 availh = wallpaperWinBottom - wallpaperWinTop - dh;
    offset = availh > 0 ? - (Int32)(availh * wpy + 0.5) : 0;
    if (mLastWallpaperDisplayOffsetY != Elastos::Core::Math::INT32_MIN_VALUE) {
        offset += mLastWallpaperDisplayOffsetY;
    }
    if (wallpaperWin->mYOffset != offset) {
        if (DEBUG_WALLPAPER) Slogger::V(TAG, "Update wallpaper %p y: %d",
                wallpaperWin, offset);
        changed = TRUE;
        wallpaperWin->mYOffset = offset;
    }
    if (wallpaperWin->mWallpaperY != wpy || wallpaperWin->mWallpaperYStep != wpys) {
        wallpaperWin->mWallpaperY = wpy;
        wallpaperWin->mWallpaperYStep = wpys;
        rawChanged = TRUE;
    }

    if (rawChanged) {
        // try {
        if (DEBUG_WALLPAPER) Slogger::V(TAG, "Report new wp offset %p x=%f y=%f",
                wallpaperWin, wallpaperWin->mWallpaperX,
                wallpaperWin->mWallpaperY);
        if (sync) {
            mWaitingOnWallpaper = wallpaperWin;
        }
        ECode ec = wallpaperWin->mClient->DispatchWallpaperOffsets(
                wallpaperWin->mWallpaperX, wallpaperWin->mWallpaperY,
                wallpaperWin->mWallpaperXStep, wallpaperWin->mWallpaperYStep, sync);
        if (FAILED(ec)) {
            return changed;
        }
        if (sync) {
            if (mWaitingOnWallpaper != NULL) {
                Int64 start = SystemClock::GetUptimeMillis();
                if ((mLastWallpaperTimeoutTime + WALLPAPER_TIMEOUT_RECOVERY)
                        < start) {
                    // try {
                    if (DEBUG_WALLPAPER) Slogger::V(TAG,
                            "Waiting for offset complete...");
                    mWindowMapLock.Wait(WALLPAPER_TIMEOUT);
                    // } catch (InterruptedException e) {
                    // }
                    if (DEBUG_WALLPAPER) Slogger::V(TAG, "Offset complete!");
                    if ((start + WALLPAPER_TIMEOUT)
                            < SystemClock::GetUptimeMillis()) {
                        Slogger::I(TAG, "Timeout waiting for wallpaper to offset: %p",
                                wallpaperWin);
                        mLastWallpaperTimeoutTime = start;
                    }
                }
                mWaitingOnWallpaper = NULL;
            }
        }
        // } catch (RemoteException e) {
        // }
    }

    return changed;
}

void CWindowManagerService::WallpaperOffsetsComplete(
    /* [in] */ IBinder* window)
{
    synchronized (mWindowMapLock) {
        if (mWaitingOnWallpaper != NULL &&
                IBinder::Probe(mWaitingOnWallpaper->mClient.Get()) == window) {
            mWaitingOnWallpaper = NULL;
            mWindowMapLock.NotifyAll();
        }
    }
}

void CWindowManagerService::UpdateWallpaperOffsetLocked(
    /* [in] */ WindowState* changingTarget,
    /* [in] */ Boolean sync)
{
    AutoPtr<DisplayContent> displayContent = changingTarget->GetDisplayContent();
    if (displayContent == NULL) {
        return;
    }
    AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
    Int32 dw, dh;
    displayInfo->GetLogicalWidth(&dw);
    displayInfo->GetLogicalHeight(&dh);

    AutoPtr<WindowState> target = mWallpaperTarget;
    if (target != NULL) {
        if (target->mWallpaperX >= 0) {
            mLastWallpaperX = target->mWallpaperX;
        }
        else if (changingTarget->mWallpaperX >= 0) {
            mLastWallpaperX = changingTarget->mWallpaperX;
        }
        if (target->mWallpaperY >= 0) {
            mLastWallpaperY = target->mWallpaperY;
        }
        else if (changingTarget->mWallpaperY >= 0) {
            mLastWallpaperY = changingTarget->mWallpaperY;
        }
        if (target->mWallpaperDisplayOffsetX != Elastos::Core::Math::INT32_MIN_VALUE) {
            mLastWallpaperDisplayOffsetX = target->mWallpaperDisplayOffsetX;
        }
        else if (changingTarget->mWallpaperDisplayOffsetX != Elastos::Core::Math::INT32_MIN_VALUE) {
            mLastWallpaperDisplayOffsetX = changingTarget->mWallpaperDisplayOffsetX;
        }
        if (target->mWallpaperDisplayOffsetY != Elastos::Core::Math::INT32_MIN_VALUE) {
            mLastWallpaperDisplayOffsetY = target->mWallpaperDisplayOffsetY;
        }
        else if (changingTarget->mWallpaperDisplayOffsetY != Elastos::Core::Math::INT32_MIN_VALUE) {
            mLastWallpaperDisplayOffsetY = changingTarget->mWallpaperDisplayOffsetY;
        }
    }

    List< AutoPtr<WindowToken> >::ReverseIterator rit;
    for (rit = mWallpaperTokens.RBegin(); rit != mWallpaperTokens.REnd(); ++rit) {
        AutoPtr<WindowToken> token = *rit;
        WindowList::ReverseIterator wRIt;
        for (wRIt = token->mWindows.RBegin(); wRIt != token->mWindows.REnd(); ++wRIt) {
            AutoPtr<WindowState> wallpaper = *wRIt;
            if (UpdateWallpaperOffsetLocked(wallpaper, dw, dh, sync)) {
                AutoPtr<WindowStateAnimator> winAnimator = wallpaper->mWinAnimator;
                winAnimator->ComputeShownFrameLocked();
                // No need to lay out the windows - we can just set the wallpaper position
                // directly.
                winAnimator->SetWallpaperOffset(wallpaper->mShownFrame);
                // We only want to be synchronous with one wallpaper.
                sync = FALSE;
            }
        }
    }
}

void CWindowManagerService::DispatchWallpaperVisibility(
    /* [in] */ WindowState* wallpaper,
    /* [in] */ Boolean visible)
{
    if (wallpaper->mWallpaperVisible != visible) {
        wallpaper->mWallpaperVisible = visible;
        //try {
        // if (DEBUG_VISIBILITY || DEBUG_WALLPAPER_LIGHT) Slog.v(TAG,
        //         "Updating vis of wallpaper " + wallpaper
        //         + ": " + visible + " from:\n" + Debug.getCallers(4, "  "));
        wallpaper->mClient->DispatchAppVisibility(visible);
        // } catch (RemoteException e) {
        // }
    }
}

void CWindowManagerService::UpdateWallpaperVisibilityLocked()
{
    Boolean visible = IsWallpaperVisible(mWallpaperTarget);
    AutoPtr<DisplayContent> displayContent = mWallpaperTarget->GetDisplayContent();
    if (displayContent == NULL) {
        return;
    }
    AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
    Int32 dw, dh;
    displayInfo->GetLogicalWidth(&dw);
    displayInfo->GetLogicalHeight(&dh);

    List< AutoPtr<WindowToken> >::ReverseIterator rit = mWallpaperTokens.RBegin();
    for (; rit != mWallpaperTokens.REnd(); ++rit) {
        AutoPtr<WindowToken> token = *rit;
        if (token->mHidden == visible) {
            token->mHidden = !visible;
            // Need to do a layout to ensure the wallpaper now has the
            // correct size.
            GetDefaultDisplayContentLocked()->mLayoutNeeded = TRUE;
        }

        WindowList::ReverseIterator wRIt = token->mWindows.RBegin();
        for (; wRIt != token->mWindows.REnd(); ++wRIt) {
            AutoPtr<WindowState> wallpaper = *wRIt;
            if (visible) {
                UpdateWallpaperOffsetLocked(wallpaper, dw, dh, FALSE);
            }

            DispatchWallpaperVisibility(wallpaper, visible);
        }
    }
}

Int32 CWindowManagerService::AddWindow(
    /* [in] */ CSession* session,
    /* [in] */ IIWindow* client,
    /* [in] */ Int32 seq,
    /* [in] */ IWindowManagerLayoutParams* attrs,
    /* [in] */ Int32 viewVisibility,
    /* [in] */ Int32 displayId,
    /* [in] */ IRect* inContentInsets,
    /* [in] */ IInputChannel* inInputChannel,
    /* [out] */ IRect** outContentInsets,
    /* [out] */ IInputChannel** outInputChannel)
{
    AutoPtr< ArrayOf<Int32> > appOp = ArrayOf<Int32>::Alloc(1);
    Int32 res;
    mPolicy->CheckAddPermission(attrs, appOp, &res);
    if (res != IWindowManagerGlobal::ADD_OKAY) {
        return res;
    }

    Boolean reportNewConfig = FALSE;
    AutoPtr<WindowState> attachedWindow;
    AutoPtr<WindowState> win;
    Int64 origId;
    Int32 type;
    attrs->GetType(&type);

    synchronized (mWindowMapLock) {
        assert(mDisplayReady);
        // if (!mDisplayReady) {
        //     throw new IllegalStateException("Display has not been initialialized");
        // }

        AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
        if (displayContent == NULL) {
            Slogger::W(TAG, "Attempted to add window to a display that does not exist: %d.  Aborting."
                    , displayId);
            return IWindowManagerGlobal::ADD_INVALID_DISPLAY;
        }
        if (!displayContent->HasAccess(session->mUid)) {
            Slogger::W(TAG, "Attempted to add window to a display for which the application does not have access: %d.  Aborting."
                    , displayId);
            return IWindowManagerGlobal::ADD_INVALID_DISPLAY;
        }

        HashMap<AutoPtr<IBinder>, AutoPtr<WindowState> >::Iterator it
                = mWindowMap.Find(IBinder::Probe(client));
        if (it != mWindowMap.End() && it->mSecond != NULL) {
            Slogger::W(TAG, "Window %p is already added", client);
            return IWindowManagerGlobal::ADD_DUPLICATE_ADD;
        }

        AutoPtr<IBinder> attrsToken;
        attrs->GetToken((IBinder**)&attrsToken);
        if (type >= IWindowManagerLayoutParams::FIRST_SUB_WINDOW
                && type <= IWindowManagerLayoutParams::LAST_SUB_WINDOW) {
            WindowForClientLocked(NULL, attrsToken, FALSE, (WindowState**)&attachedWindow);
            if (attachedWindow == NULL) {
                Slogger::W(TAG,
                        "Attempted to add window with token that is not a window: %p.  Aborting."
                        , attrsToken.Get());
                return IWindowManagerGlobal::ADD_BAD_SUBWINDOW_TOKEN;
            }
            Int32 attachedWinType;
            attachedWindow->mAttrs->GetType(&attachedWinType);
            if (attachedWinType >= IWindowManagerLayoutParams::FIRST_SUB_WINDOW
                    && attachedWinType <= IWindowManagerLayoutParams::LAST_SUB_WINDOW) {
                Slogger::W(TAG,
                        "Attempted to add window with token that is a sub-window: %p.  Aborting."
                        , attrsToken.Get());
                return IWindowManagerGlobal::ADD_BAD_SUBWINDOW_TOKEN;
            }
        }

        if (type == IWindowManagerLayoutParams::TYPE_PRIVATE_PRESENTATION && !displayContent->IsPrivate()) {
            Slogger::W(TAG, "Attempted to add private presentation window to a non-private display.  Aborting.");
            return IWindowManagerGlobal::ADD_PERMISSION_DENIED;
        }
        Boolean addToken = FALSE;
        AutoPtr<WindowToken> token;
        if (attrsToken != NULL) {
            HashMap<AutoPtr<IBinder>, AutoPtr<WindowToken> >::Iterator tokenIt
                    = mTokenMap.Find(attrsToken);
            if (tokenIt != mTokenMap.End()) {
                token = tokenIt->mSecond;
            }
        }
        if (token == NULL) {
            if (type >= IWindowManagerLayoutParams::FIRST_APPLICATION_WINDOW
                    && type <= IWindowManagerLayoutParams::LAST_APPLICATION_WINDOW) {
                Slogger::W(TAG,
                        "Attempted to add application window with unknown token %p.  Aborting."
                        , attrsToken.Get());
                return IWindowManagerGlobal::ADD_BAD_APP_TOKEN;
            }
            if (type == IWindowManagerLayoutParams::TYPE_INPUT_METHOD) {
                Slogger::W(TAG,
                        "Attempted to add input method window with unknown token %p.  Aborting."
                        , attrsToken.Get());
                return IWindowManagerGlobal::ADD_BAD_APP_TOKEN;
            }
            if (type == IWindowManagerLayoutParams::TYPE_VOICE_INTERACTION) {
                Slogger::W(TAG, "Attempted to add voice interaction window with unknown token %p.  Aborting."
                        , attrsToken.Get());
                return IWindowManagerGlobal::ADD_BAD_APP_TOKEN;
            }
            if (type == IWindowManagerLayoutParams::TYPE_WALLPAPER) {
                Slogger::W(TAG,
                        "Attempted to add wallpaper window with unknown token %p.  Aborting."
                        , attrsToken.Get());
                return IWindowManagerGlobal::ADD_BAD_APP_TOKEN;
            }
            if (type == IWindowManagerLayoutParams::TYPE_DREAM) {
                Slogger::W(TAG,
                        "Attempted to add Dream window with unknown token %p.  Aborting."
                        , attrsToken.Get());
                return IWindowManagerGlobal::ADD_BAD_APP_TOKEN;
            }
            token = new WindowToken(this, attrsToken, -1, FALSE);
            addToken = TRUE;
        }
        else if (type >= IWindowManagerLayoutParams::FIRST_APPLICATION_WINDOW
                && type <= IWindowManagerLayoutParams::LAST_APPLICATION_WINDOW) {
            AutoPtr<AppWindowToken> atoken = token->mAppWindowToken;
            if (atoken == NULL) {
                Slogger::W(TAG,
                    "Attempted to add window with non-application token %p.  Aborting."
                    , token.Get());
                return IWindowManagerGlobal::ADD_NOT_APP_TOKEN;
            }
            else if (atoken->mRemoved) {
                Slogger::W(TAG,
                        "Attempted to add window with exiting application token %p.  Aborting."
                        , token.Get());
                return IWindowManagerGlobal::ADD_APP_EXITING;
            }
            if (type == IWindowManagerLayoutParams::TYPE_APPLICATION_STARTING
                    && atoken->mFirstWindowDrawn) {
                // No need for this guy!
                AutoPtr<ICharSequence> title;
                attrs->GetTitle((ICharSequence**)&title);
                String str;
                title->ToString(&str);
                if (localLOGV) Slogger::V(TAG,
                        TAG, "**** NO NEED TO START: %s", str.string());
                return IWindowManagerGlobal::ADD_STARTING_NOT_NEEDED;
            }
        }
        else if (type == IWindowManagerLayoutParams::TYPE_INPUT_METHOD) {
            if (token->mWindowType != IWindowManagerLayoutParams::TYPE_INPUT_METHOD) {
                Slogger::W(TAG,
                        "Attempted to add input method window with bad token %p.  Aborting."
                        , attrsToken.Get());
                return IWindowManagerGlobal::ADD_BAD_APP_TOKEN;
            }
        }
        else if (type == IWindowManagerLayoutParams::TYPE_VOICE_INTERACTION) {
            if (token->mWindowType != IWindowManagerLayoutParams::TYPE_VOICE_INTERACTION) {
                Slogger::W(TAG, "Attempted to add voice interaction window with bad token %p.  Aborting."
                        , attrsToken.Get());
                  return IWindowManagerGlobal::ADD_BAD_APP_TOKEN;
            }
        }
        else if (type == IWindowManagerLayoutParams::TYPE_WALLPAPER) {
            if (token->mWindowType != IWindowManagerLayoutParams::TYPE_WALLPAPER) {
                Slogger::W(TAG,
                        "Attempted to add wallpaper window with bad token %p.  Aborting."
                        , attrsToken.Get());
                return IWindowManagerGlobal::ADD_BAD_APP_TOKEN;
            }
        }
        else if (type == IWindowManagerLayoutParams::TYPE_DREAM) {
            if (token->mWindowType != IWindowManagerLayoutParams::TYPE_DREAM) {
                Slogger::W(TAG, "Attempted to add Dream window with bad token %p.  Aborting."
                        , attrsToken.Get());
                return IWindowManagerGlobal::ADD_BAD_APP_TOKEN;
            }
        }
        else if (token->mAppWindowToken != NULL) {
            Slogger::W(TAG, "Non-null appWindowToken for system window of type=%d", type);
            // It is not valid to use an app token with other system types; we will
            // instead make a new token for it (as if null had been passed in for the token).
            attrs->SetToken(NULL);
            token = new WindowToken(this, NULL, -1, FALSE);
            addToken = TRUE;
        }

        win = new WindowState(this, session, client, token,
                attachedWindow, (*appOp)[0],seq, attrs, viewVisibility, displayContent);
        if (win->mDeathRecipient == NULL) {
            // Client has apparently died, so there is no reason to
            // continue.
            Slogger::W(TAG, "Adding window client %p that is dead, aborting.", client);
            return IWindowManagerGlobal::ADD_APP_EXITING;
        }

        if (win->GetDisplayContent() == NULL) {
            Slogger::W(TAG, "Adding window to Display that has been removed.");
            return IWindowManagerGlobal::ADD_INVALID_DISPLAY;
        }

        mPolicy->AdjustWindowParamsLw(win->mAttrs);
        Boolean result;
        mPolicy->CheckShowToOwnerOnly(attrs, &result);
        win->SetShowToOwnerOnlyLocked(result);

        mPolicy->PrepareAddWindowLw(win, attrs, &res);
        if (res != IWindowManagerGlobal::ADD_OKAY) {
            return res;
        }

        Int32 inputFeatures;
        attrs->GetInputFeatures(&inputFeatures);
        if (outInputChannel != NULL && (inputFeatures
                & IWindowManagerLayoutParams::INPUT_FEATURE_NO_INPUT_CHANNEL) == 0) {
            String name = win->MakeInputChannelName();
            AutoPtr<IInputChannelHelper> helper;
            CInputChannelHelper::AcquireSingleton((IInputChannelHelper**)&helper);
            AutoPtr<ArrayOf<IInputChannel*> > inputChannels;
            ASSERT_SUCCEEDED(helper->OpenInputChannelPair(name, (ArrayOf<IInputChannel*>**)&inputChannels));
            win->SetInputChannel((*inputChannels)[0]);
            CInputChannel::New(outInputChannel);
            (*inputChannels)[1]->TransferTo(*outInputChannel);

            ASSERT_SUCCEEDED(mInputManager->RegisterInputChannel(win->mInputChannel,
                    win->mInputWindowHandle))
        }

        // From now on, no exceptions or errors allowed!

        res = IWindowManagerGlobal::ADD_OKAY;

        origId = Binder::ClearCallingIdentity();

        if (addToken) {
            if (attrsToken != NULL)
                mTokenMap[attrsToken] = token;
        }
        win->Attach();
        mWindowMap[IBinder::Probe(client)] = win;
        if (win->mAppOp != IAppOpsManager::OP_NONE) {
            Int32 uid;
            win->GetOwningUid(&uid);
            String packageName;
            win->GetOwningPackage(&packageName);
            Int32 result;
            if (mAppOps->StartOpNoThrow(win->mAppOp, uid, packageName, &result), result != IAppOpsManager::MODE_ALLOWED) {
                win->SetAppOpVisibilityLw(FALSE);
            }
        }

        if (type == IWindowManagerLayoutParams::TYPE_APPLICATION_STARTING &&
                token->mAppWindowToken != NULL) {
            token->mAppWindowToken->mStartingWindow = win;
            if (DEBUG_STARTING_WINDOW) {
                Slogger::V (TAG, "addWindow: %p startingWindow=%p",
                    token->mAppWindowToken, win.Get());
            }
        }

        Boolean imMayMove = TRUE;

        if (type == IWindowManagerLayoutParams::TYPE_INPUT_METHOD) {
            win->mGivenInsetsPending = TRUE;
            mInputMethodWindow = win;
            AddInputMethodWindowToListLocked(win);
            imMayMove = FALSE;
        }
        else if (type == IWindowManagerLayoutParams::TYPE_INPUT_METHOD_DIALOG) {
            mInputMethodDialogs.PushBack(win);
            AddWindowToListInOrderLocked(win, TRUE);
            MoveInputMethodDialogsLocked(FindDesiredInputMethodWindowItLocked(TRUE));
            imMayMove = FALSE;
        }
        else {
            AddWindowToListInOrderLocked(win, TRUE);
            if (type == IWindowManagerLayoutParams::TYPE_WALLPAPER) {
                mLastWallpaperTimeoutTime = 0;
                displayContent->mPendingLayoutChanges |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_WALLPAPER;
            }
            else {
                Int32 attrsFlags;
                attrs->GetFlags(&attrsFlags);
                if ((attrsFlags & IWindowManagerLayoutParams::FLAG_SHOW_WALLPAPER) != 0) {
                    displayContent->mPendingLayoutChanges |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_WALLPAPER;
                }
                else if (mWallpaperTarget != NULL
                        && mWallpaperTarget->mLayer >= win->mBaseLayer) {
                    // If there is currently a wallpaper being shown, and
                    // the base layer of the new window is below the current
                    // layer of the target window, then adjust the wallpaper.
                    // This is to avoid a new window being placed between the
                    // wallpaper and its target.
                    displayContent->mPendingLayoutChanges |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_WALLPAPER;
                }
            }
        }

        win->mWinAnimator->mEnterAnimationPending = TRUE;

        if (displayContent->mIsDefaultDisplay) {
            mPolicy->GetContentInsetHintLw(attrs, inContentInsets);
        }
        else {
            inContentInsets->SetEmpty();
        }

        if (mInTouchMode) {
            res |= IWindowManagerGlobal::ADD_FLAG_IN_TOUCH_MODE;
        }
        if (win->mAppToken == NULL || !win->mAppToken->mClientHidden) {
            res |= IWindowManagerGlobal::ADD_FLAG_APP_VISIBLE;
        }

        Boolean focusChanged = FALSE;
        if (win->CanReceiveKeys()) {
            focusChanged = UpdateFocusedWindowLocked(UPDATE_FOCUS_WILL_ASSIGN_LAYERS,
                    FALSE /*updateInputWindows*/);
            if (focusChanged) {
                imMayMove = FALSE;
            }
        }

        if (imMayMove) {
            MoveInputMethodWindowsIfNeededLocked(FALSE);
        }

        AssignLayersLocked(displayContent->GetWindowList());
        // Don't do layout here, the window must call
        // relayout to be displayed, so we'll do it there.

        if (focusChanged) {
            mInputMonitor->SetInputFocusLw(mCurrentFocus, FALSE /*updateInputWindows*/);
        }
        mInputMonitor->UpdateInputWindowsLw(FALSE /*force*/);

        // if (localLOGV) Slog.v(
        //         TAG, "New client " + client.asBinder()
        //         + ": window=" + win);

        if (win->IsVisibleOrAdding() && UpdateOrientationFromAppTokensLocked(FALSE)) {
            reportNewConfig = TRUE;
        }
    }

    if (reportNewConfig) {
       SendNewConfiguration();
    }
    Binder::RestoreCallingIdentity(origId);

    *outContentInsets = inContentInsets;
    REFCOUNT_ADD(*outContentInsets);
    return res;
}

Boolean CWindowManagerService::IsScreenCaptureDisabledLocked(
    /* [in] */ Int32 userId)
{
    AutoPtr<IInterface> temp;
    mScreenCaptureDisabled->Get(userId, (IInterface**)&temp);
    AutoPtr<IBoolean> disabled = IBoolean::Probe(temp);
    if (disabled == NULL) {
        return FALSE;
    }
    Boolean value;
    disabled->GetValue(&value);
    return value;
}

ECode CWindowManagerService::SetScreenCaptureDisabled(
    /* [in] */ Int32 userId,
    /* [in] */ Boolean disabled)
{
    Int32 callingUid = Binder::GetCallingUid();
    if (callingUid != IProcess::SYSTEM_UID) {
        Slogger::E(TAG, "Only system can call setScreenCaptureDisabled.");
    }

    synchronized (mWindowMapLock) {
        AutoPtr<IBoolean> b;
        CBoolean::New(disabled, (IBoolean**)&b);
        mScreenCaptureDisabled->Put(userId, b);
    }
    return NOERROR;
}

void CWindowManagerService::RemoveWindow(
    /* [in] */ CSession* session,
    /* [in] */ IIWindow* client)
{
    synchronized (mWindowMapLock) {
        AutoPtr<WindowState> win;
        WindowForClientLocked(session, client, FALSE, (WindowState**)&win);
        if (win == NULL) {
            return;
        }
        RemoveWindowLocked(session, win);
    }
}

void CWindowManagerService::RemoveWindowLocked(
    /* [in] */ CSession* session,
    /* [in] */ WindowState* win)
{
    Int32 type;
    if (win->mAttrs->GetType(&type), type == IWindowManagerLayoutParams::TYPE_APPLICATION_STARTING) {
        if (DEBUG_STARTING_WINDOW) Slogger::D(TAG, "Starting window removed %p", win);
    }
    // if (localLOGV || DEBUG_FOCUS || DEBUG_FOCUS_LIGHT && win==mCurrentFocus) Slog.v(
    //         TAG, "Remove " + win + " client="
    //         + Integer.toHexString(System.identityHashCode(win.mClient.asBinder()))
    //         + ", surface=" + win.mWinAnimator.mSurfaceControl + " Callers="
    //         + Debug.getCallers(4));

    Int64 origId = Binder::ClearCallingIdentity();

    win->DisposeInputChannel();

    // if (DEBUG_APP_TRANSITIONS) Slog.v(
    //         TAG, "Remove " + win + ": mSurface=" + win.mWinAnimator.mSurfaceControl
    //         + " mExiting=" + win.mExiting
    //         + " isAnimating=" + win.mWinAnimator.isAnimating()
    //         + " app-animation="
    //         + (win.mAppToken != null ? win.mAppToken.mAppAnimator.animation : null)
    //         + " inPendingTransaction="
    //         + (win.mAppToken != null ? win.mAppToken.inPendingTransaction : false)
    //         + " mDisplayFrozen=" + mDisplayFrozen);
    // Visibility of the removed window. Will be used later to update orientation later on.
    Boolean wasVisible = FALSE;
    // First, see if we need to run an animation.  If we do, we have
    // to hold off on removing the window until the animation is done.
    // If the display is frozen, just remove immediately, since the
    // animation wouldn't be seen.
    if (win->mHasSurface && OkToDisplay()) {
        // If we are not currently running the exit animation, we
        // need to see about starting one.
        wasVisible = win->IsWinVisibleLw();
        if (wasVisible) {
            Int32 transit = IWindowManagerPolicy::TRANSIT_EXIT;
            Int32 type;
            win->mAttrs->GetType(&type);
            if (type == IWindowManagerLayoutParams::TYPE_APPLICATION_STARTING) {
                transit = IWindowManagerPolicy::TRANSIT_PREVIEW_DONE;
            }
            // Try starting an animation.
            if (win->mWinAnimator->ApplyAnimationLocked(transit, FALSE)) {
                win->mExiting = TRUE;
            }
            //TODO (multidisplay): Magnification is supported only for the default display.
            if (mAccessibilityController != NULL
                    && win->GetDisplayId() == IDisplay::DEFAULT_DISPLAY) {
                mAccessibilityController->OnWindowTransitionLocked(win, transit);
            }
        }
        if (win->mExiting || win->mWinAnimator->IsAnimating()) {
            // The exit animation is running... wait for it!
            //Slog.i(TAG, "*** Running exit animation...");
            win->mExiting = TRUE;
            win->mRemoveOnExit = TRUE;
            AutoPtr<DisplayContent> displayContent = win->GetDisplayContent();
            if (displayContent != NULL) {
                displayContent->mLayoutNeeded = TRUE;
            }
            Boolean focusChanged = UpdateFocusedWindowLocked(
                    UPDATE_FOCUS_WILL_PLACE_SURFACES, FALSE /*updateInputWindows*/);
            PerformLayoutAndPlaceSurfacesLocked();
            if (win->mAppToken != NULL) {
                win->mAppToken->UpdateReportedVisibilityLocked();
            }
            if (focusChanged) {
                mInputMonitor->UpdateInputWindowsLw(FALSE /*force*/);
            }
            //dump();
            Binder::RestoreCallingIdentity(origId);
            return;
        }
    }

    RemoveWindowInnerLocked(session, win);
    // Removing a visible window will effect the computed orientation
    // So just update orientation if needed.
    if (wasVisible && UpdateOrientationFromAppTokensLocked(FALSE)) {
        Boolean result;
        mH->SendEmptyMessage(H::SEND_NEW_CONFIGURATION, &result);
    }
    UpdateFocusedWindowLocked(UPDATE_FOCUS_NORMAL, TRUE /*updateInputWindows*/);
    Binder::RestoreCallingIdentity(origId);
}

void CWindowManagerService::RemoveWindowInnerLocked(
    /* [in] */ CSession* session,
    /* [in] */ WindowState* win)
{
    if (win->mRemoved) {
        // Nothing to do.
        return;
    }

    WindowList::ReverseIterator rit = win->mChildWindows.RBegin();
    for (; rit != win->mChildWindows.REnd(); ++rit) {
        AutoPtr<WindowState> cwin = *rit;
        // Slog.w(TAG, "Force-removing child win " + cwin + " from container "
        //         + win);
        RemoveWindowInnerLocked(cwin->mSession, cwin);
    }

    win->mRemoved = TRUE;

    if (mInputMethodTarget.Get() == win) {
        MoveInputMethodWindowsIfNeededLocked(FALSE);
    }

    // if (false) {
    //     RuntimeException e = new RuntimeException("here");
    //     e.fillInStackTrace();
    //     Slog.w(TAG, "Removing window " + win, e);
    // }

    mPolicy->RemoveWindowLw(win);
    win->RemoveLocked();

    if (DEBUG_ADD_REMOVE) Slogger::V(TAG, "removeWindowInnerLocked: %p", win);
    mWindowMap.Erase(IBinder::Probe(win->mClient.Get()));
    if (win->mAppOp != IAppOpsManager::OP_NONE) {
        Int32 uid;
        win->GetOwningUid(&uid);
        String packageName;
        win->GetOwningPackage(&packageName);
        mAppOps->FinishOp(win->mAppOp, uid, packageName);
    }

    mPendingRemove.Remove(win);
    mResizingWindows.Remove(win);
    mWindowsChanged = TRUE;
    if (DEBUG_WINDOW_MOVEMENT) Slogger::V(TAG, "Final remove of window: %p", win);

    Int32 winType, winFlags;
    if (mInputMethodWindow.Get() == win) {
        mInputMethodWindow = NULL;
    }
    else {
        win->mAttrs->GetType(&winType);
        if (winType == IWindowManagerLayoutParams::TYPE_INPUT_METHOD_DIALOG) {
            mInputMethodDialogs.Remove(win);
        }
    }

    AutoPtr<WindowToken> token = win->mToken;
    AutoPtr<AppWindowToken> atoken = win->mAppToken;
    if (DEBUG_ADD_REMOVE) Slogger::V(TAG, "Removing %p from %p", win, token.Get());
    token->mWindows.Remove(win);
    if (atoken != NULL) {
        atoken->mAllAppWindows.Remove(win);
    }
    // if (localLOGV) Slog.v(
    //         TAG, "**** Removing window " + win + ": count="
    //         + token.windows.size());
    if (token->mWindows.Begin() == token->mWindows.End()) {
        if (!token->mExplicit && token->mToken != NULL) {
            mTokenMap.Erase(token->mToken);
        }
        else if (atoken != NULL) {
            atoken->mFirstWindowDrawn = FALSE;
        }
    }

    if (atoken != NULL) {
        if (atoken->mStartingWindow.Get() == win) {
            atoken->mStartingWindow = NULL;
        }
        else if (atoken->mAllAppWindows.IsEmpty() && atoken->mStartingData != NULL) {
            // If this is the last window and we had requested a starting
            // transition window, well there is no point now.
            if (DEBUG_STARTING_WINDOW) {
                Slogger::V(TAG, "Nulling last startingWindow");
            }
            atoken->mStartingData = NULL;
        }
        else if (atoken->mAllAppWindows.GetSize() == 1 && atoken->mStartingView != NULL) {
            // If this is the last window except for a starting transition
            // window, we need to get rid of the starting transition.
            ScheduleRemoveStartingWindowLocked(atoken);
        }
    }

    win->mAttrs->GetType(&winType);
    if (winType == IWindowManagerLayoutParams::TYPE_WALLPAPER) {
        mLastWallpaperTimeoutTime = 0;
        GetDefaultDisplayContentLocked()->mPendingLayoutChanges |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_WALLPAPER;
    }
    else {
        win->mAttrs->GetFlags(&winFlags);
        if ((winFlags & IWindowManagerLayoutParams::FLAG_SHOW_WALLPAPER) != 0) {
            GetDefaultDisplayContentLocked()->mPendingLayoutChanges |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_WALLPAPER;
        }
    }

    AutoPtr<WindowList> windows = win->GetWindowList();
    if (windows != NULL) {
        windows->Remove(win);
        if (!mInLayout) {
            AssignLayersLocked(windows);
            AutoPtr<DisplayContent> displayContent = win->GetDisplayContent();
            if (displayContent != NULL) {
                displayContent->mLayoutNeeded = TRUE;
            }
            PerformLayoutAndPlaceSurfacesLocked();
            if (win->mAppToken != NULL) {
                win->mAppToken->UpdateReportedVisibilityLocked();
            }
        }
    }

    mInputMonitor->UpdateInputWindowsLw(TRUE /*force*/);
}

void CWindowManagerService::UpdateAppOpsState()
{
    synchronized (mWindowMapLock) {
        Int32 numDisplays;
        mDisplayContents->GetSize(&numDisplays);
        for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
            AutoPtr<IInterface> value;
            mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
            AutoPtr<DisplayContent> dc = (DisplayContent*)(IObject*)value.Get();
            AutoPtr<WindowList> windows = dc->GetWindowList();
            WindowList::Iterator it = windows->Begin();
            for (; it != windows->End(); ++it) {
                AutoPtr<WindowState> win = *it;
                if (win->mAppOp != IAppOpsManager::OP_NONE) {
                    Int32 uid;
                    win->GetOwningUid(&uid);
                    String packageName;
                    win->GetOwningPackage(&packageName);
                    Int32 mode;
                    mAppOps->CheckOpNoThrow(win->mAppOp, uid, packageName, &mode);
                    win->SetAppOpVisibilityLw(mode == IAppOpsManager::MODE_ALLOWED);
                }
            }
        }
    }
}

// void CWindowManagerService::LogSurface(
//     /* [in] */ WindowState* w,
//     /* [in] */ const String& msg,
//     /* [in] */ RuntimeException where)
// {
//     String str = "  SURFACE " + msg + ": " + w;
//     if (where != null) {
//         Slog.i(TAG, str, where);
//     } else {
//         Slog.i(TAG, str);
//     }
// }

// static void logSurface(SurfaceControl s, String title, String msg, RuntimeException where) {
//     String str = "  SURFACE " + s + ": " + msg + " / " + title;
//     if (where != null) {
//         Slog.i(TAG, str, where);
//     } else {
//         Slog.i(TAG, str);
//     }
// }

void CWindowManagerService::SetTransparentRegionWindow(
    /* [in] */ CSession* session,
    /* [in] */ IIWindow* client,
    /* [in] */ IRegion* region)
{
    Int64 origId = Binder::ClearCallingIdentity();
    // try {
    synchronized (mWindowMapLock) {

        AutoPtr<WindowState> w;
        WindowForClientLocked(session, client, FALSE, (WindowState**)&w);
        if (w != NULL && w->mHasSurface) {
            w->mWinAnimator->SetTransparentRegionHintLocked(region);
        }
        // } finally {
        Binder::RestoreCallingIdentity(origId);
        // }
    }
}

void CWindowManagerService::SetInsetsWindow(
    /* [in] */ CSession* session,
    /* [in] */ IIWindow* client,
    /* [in] */ Int32 touchableInsets,
    /* [in] */ IRect* contentInsets,
    /* [in] */ IRect* visibleInsets,
    /* [in] */ IRegion* touchableRegion)
{
    Int64 origId = Binder::ClearCallingIdentity();
    // try {
    synchronized (mWindowMapLock) {

        AutoPtr<WindowState> w;
        WindowForClientLocked(session, client, FALSE, (WindowState**)&w);
        if (w != NULL) {
            w->mGivenInsetsPending = FALSE;
            w->mGivenContentInsets->Set(contentInsets);
            w->mGivenVisibleInsets->Set(visibleInsets);
            Boolean result;
            w->mGivenTouchableRegion->Set(touchableRegion, &result);
            w->mTouchableInsets = touchableInsets;
            if (w->mGlobalScale != 1) {
                w->mGivenContentInsets->Scale(w->mGlobalScale);
                w->mGivenVisibleInsets->Scale(w->mGlobalScale);
                w->mGivenTouchableRegion->Scale(w->mGlobalScale);
            }
            AutoPtr<DisplayContent> displayContent = w->GetDisplayContent();
            if (displayContent != NULL) {
                displayContent->mLayoutNeeded = TRUE;
            }
            PerformLayoutAndPlaceSurfacesLocked();
        }
        // } finally {
        Binder::RestoreCallingIdentity(origId);
        // }
    }
}

void CWindowManagerService::GetWindowDisplayFrame(
    /* [in] */ CSession* session,
    /* [in] */ IIWindow* client,
    /* [in] */ IRect* outDisplayFrame)
{
    synchronized (mWindowMapLock) {
        AutoPtr<WindowState> win;
        WindowForClientLocked(session, client, FALSE, (WindowState**)&win);
        if (win == NULL) {
            outDisplayFrame->SetEmpty();
            return;
        }
        outDisplayFrame->Set(win->mDisplayFrame);
    }
}

void CWindowManagerService::SetWindowWallpaperPositionLocked(
    /* [in] */ WindowState* window,
    /* [in] */ Float x,
    /* [in] */ Float y,
    /* [in] */ Float xStep,
    /* [in] */ Float yStep)
{
    if (window->mWallpaperX != x || window->mWallpaperY != y)  {
        window->mWallpaperX = x;
        window->mWallpaperY = y;
        window->mWallpaperXStep = xStep;
        window->mWallpaperYStep = yStep;
        UpdateWallpaperOffsetLocked(window, TRUE);
    }
}

void CWindowManagerService::WallpaperCommandComplete(
    /* [in] */ IBinder* window,
    /* [in] */ IBundle* result)
{
    synchronized (mWindowMapLock) {
        if (mWaitingOnWallpaper != NULL &&
                IBinder::Probe(mWaitingOnWallpaper->mClient.Get()) == window) {
            mWaitingOnWallpaper = NULL;
            mWindowMapLock.NotifyAll();
        }
    }
}

void CWindowManagerService::SetWindowWallpaperDisplayOffsetLocked(
    /* [in] */ WindowState* window,
    /* [in] */ Int32 x,
    /* [in] */ Int32 y)
{
    if (window->mWallpaperDisplayOffsetX != x || window->mWallpaperDisplayOffsetY != y)  {
        window->mWallpaperDisplayOffsetX = x;
        window->mWallpaperDisplayOffsetY = y;
        UpdateWallpaperOffsetLocked(window, TRUE);
    }
}

AutoPtr<IBundle> CWindowManagerService::SendWindowWallpaperCommandLocked(
    /* [in] */ WindowState* window,
    /* [in] */ const String& action,
    /* [in] */ Int32 x,
    /* [in] */ Int32 y,
    /* [in] */ Int32 z,
    /* [in] */ IBundle* extras,
    /* [in] */ Boolean sync)
{
    if (window == mWallpaperTarget || window == mLowerWallpaperTarget
            || window == mUpperWallpaperTarget) {
        Boolean doWait = sync;
        List< AutoPtr<WindowToken> >::ReverseIterator rit = mWallpaperTokens.RBegin();
        for (; rit != mWallpaperTokens.REnd(); ++rit) {
            AutoPtr<WindowToken> token = *rit;
            List< AutoPtr<WindowState> >::ReverseIterator wRIt;
            for (wRIt = token->mWindows.RBegin(); wRIt != token->mWindows.REnd(); ++wRIt) {
                AutoPtr<WindowState> wallpaper = *wRIt;
                // try {
                wallpaper->mClient->DispatchWallpaperCommand(action,
                        x, y, z, extras, sync);
                // We only want to be synchronous with one wallpaper.
                sync = FALSE;
                // } catch (RemoteException e) {
                // }
            }
        }

        if (doWait) {
            // XXX Need to wait for result.
        }
    }

    return NULL;
}

void CWindowManagerService::SetUniverseTransformLocked(
    /* [in] */ WindowState* window,
    /* [in] */ Float alpha,
    /* [in] */ Float offx,
    /* [in] */ Float offy,
    /* [in] */ Float dsdx,
    /* [in] */ Float dtdx,
    /* [in] */ Float dsdy,
    /* [in] */ Float dtdy)
{
    AutoPtr<ITransformation> transform = window->mWinAnimator->mUniverseTransform;
    transform->SetAlpha(alpha);
    AutoPtr<IMatrix> matrix;
    transform->GetMatrix((IMatrix**)&matrix);
    matrix->GetValues(mTmpFloats);
    (*mTmpFloats)[IMatrix::MTRANS_X] = offx;
    (*mTmpFloats)[IMatrix::MTRANS_Y] = offy;
    (*mTmpFloats)[IMatrix::MSCALE_X] = dsdx;
    (*mTmpFloats)[IMatrix::MSKEW_Y] = dtdx;
    (*mTmpFloats)[IMatrix::MSKEW_X] = dsdy;
    (*mTmpFloats)[IMatrix::MSCALE_Y] = dtdy;
    matrix->SetValues(mTmpFloats);
    AutoPtr<DisplayContent> displayContent = window->GetDisplayContent();
    if (displayContent == NULL) {
        return;
    }
    AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
    AutoPtr<IRectF> dispRect;
    Int32 logicalW, logicalH;
    displayInfo->GetLogicalWidth(&logicalW);
    displayInfo->GetLogicalHeight(&logicalH);
    CRectF::New(0, 0, logicalW, logicalH, (IRectF**)&dispRect);
    Boolean res;
    matrix->MapRect(dispRect, &res);
    window->mGivenTouchableRegion->Set(0, 0, logicalW, logicalH, &res);
    Float left = 0, top = 0, right = 0, bottom = 0;
    dispRect->GetLeft(&left);
    dispRect->GetTop(&top);
    dispRect->GetRight(&right);
    dispRect->GetBottom(&bottom);
    window->mGivenTouchableRegion->Op((Int32)left, (Int32)top, (Int32)right,
            (Int32)bottom, Elastos::Droid::Graphics::RegionOp_DIFFERENCE, &res);
    window->mTouchableInsets = IInternalInsetsInfo::TOUCHABLE_INSETS_REGION;
    displayContent->mLayoutNeeded = TRUE;
    PerformLayoutAndPlaceSurfacesLocked();
}

void CWindowManagerService::OnRectangleOnScreenRequested(
    /* [in] */ IBinder* token,
    /* [in] */ IRect* rectangle)
{
    synchronized (mWindowMapLock) {
        if (mAccessibilityController != NULL) {
            AutoPtr<WindowState> window;
            HashMap<AutoPtr<IBinder>, AutoPtr<WindowState> >::Iterator it = mWindowMap.Find(token);
            if (it != mWindowMap.End()) {
                window = it->mSecond;
            }
            //TODO (multidisplay): Magnification is supported only for the default display.
            if (window != NULL && window->GetDisplayId() == IDisplay::DEFAULT_DISPLAY) {
                mAccessibilityController->OnRectangleOnScreenRequestedLocked(rectangle);
            }
        }
    }
}

AutoPtr<IIWindowId> CWindowManagerService::GetWindowId(
    /* [in] */ IBinder* token)
{
    AutoPtr<IIWindowId> value;
    synchronized (mWindowMapLock) {
        AutoPtr<WindowState> window;
        HashMap<AutoPtr<IBinder>, AutoPtr<WindowState> >::Iterator it = mWindowMap.Find(token);
        if (it != mWindowMap.End()) {
            window = it->mSecond;
        }
        value = window != NULL ? window->mWindowId : NULL;
    }
    return value;
}

Int32 CWindowManagerService::RelayoutWindow(
    /* [in] */ CSession* session,
    /* [in] */ IIWindow* client,
    /* [in] */ Int32 seq,
    /* [in] */ IWindowManagerLayoutParams* attrs,
    /* [in] */ Int32 requestedWidth,
    /* [in] */ Int32 requestedHeight,
    /* [in] */ Int32 viewVisibility,
    /* [in] */ Int32 flags,
    /* [in] */ IRect* inFrame,
    /* [in] */ IRect* inOverscanInsets,
    /* [in] */ IRect* inContentInsets,
    /* [in] */ IRect* inVisibleInsets,
    /* [in] */ IRect* inStableInsets,
    /* [in] */ IConfiguration* inConfig,
    /* [in] */ ISurface* inSurface,
    /* [out] */ IRect** outFrame,
    /* [out] */ IRect** outOverscanInsets,
    /* [out] */ IRect** outContentInsets,
    /* [out] */ IRect** outVisibleInsets,
    /* [out] */ IRect** outStableInsets,
    /* [out] */ IConfiguration** outConfig,
    /* [out] */ ISurface** outSurface)
{
    Boolean toBeDisplayed = FALSE;
    Boolean inTouchMode;
    Boolean configChanged;
    Boolean surfaceChanged = FALSE;
    Boolean animating;
    Int32 perm;
    mContext->CheckCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::STATUS_BAR, &perm);
    Boolean hasStatusBarPermission = (perm == IPackageManager::PERMISSION_GRANTED);

    Int64 origId = Binder::ClearCallingIdentity();

    synchronized (mWindowMapLock) {
        AutoPtr<WindowState> win;
        WindowForClientLocked(session, client, FALSE, (WindowState**)&win);
        if (win == NULL) {
            Binder::RestoreCallingIdentity(origId);
            return 0;
        }

        AutoPtr<WindowStateAnimator> winAnimator = win->mWinAnimator;
        if (win->mRequestedWidth != requestedWidth
                || win->mRequestedHeight != requestedHeight) {
            win->mLayoutNeeded = TRUE;
            win->mRequestedWidth = requestedWidth;
            win->mRequestedHeight = requestedHeight;
        }

        if (attrs != NULL) {
            mPolicy->AdjustWindowParamsLw(attrs);
        }

        // if they don't have the permission, mask out the status bar bits
        Int32 systemUiVisibility = 0;
        if (attrs != NULL) {
            Int32 attrsSystemUIVisibility, attrsSubtreeSystemUIVisibility;
            attrs->GetSystemUiVisibility(&attrsSystemUIVisibility);
            attrs->GetSubtreeSystemUiVisibility(&attrsSubtreeSystemUIVisibility);
            systemUiVisibility = (attrsSystemUIVisibility | attrsSubtreeSystemUIVisibility);
            if ((systemUiVisibility & IStatusBarManager::DISABLE_MASK) != 0) {
                if (!hasStatusBarPermission) {
                    systemUiVisibility &= ~IStatusBarManager::DISABLE_MASK;
                }
            }
        }

        if (attrs != NULL && seq == win->mSeq) {
            win->mSystemUiVisibility = systemUiVisibility;
        }

        winAnimator->mSurfaceDestroyDeferred =
                (flags & IWindowManagerGlobal::RELAYOUT_DEFER_SURFACE_DESTROY) != 0;

        Int32 winFlags = 0;
        win->mAttrs->GetFlags(&winFlags);
        Int32 attrChanges = 0;
        Int32 flagChanges = 0;
        if (attrs != NULL) {
            Int32 winType, type;
            win->mAttrs->GetType(&winType);
            attrs->GetType(&type);
            if (winType != type) {
                Binder::RestoreCallingIdentity(origId);
                return E_ILLEGAL_ARGUMENT_EXCEPTION;
                // throw new IllegalArgumentException(
                //         "Window type can not be changed after the window is added.");
            }
            Int32 attrsFlags;
            attrs->GetFlags(&attrsFlags);
            flagChanges = winFlags ^= attrsFlags;
            win->mAttrs->SetFlags(winFlags);
            win->mAttrs->CopyFrom(attrs, &attrChanges);
            if ((attrChanges & (IWindowManagerLayoutParams::LAYOUT_CHANGED
                    | IWindowManagerLayoutParams::SYSTEM_UI_VISIBILITY_CHANGED)) != 0) {
                win->mLayoutNeeded = TRUE;
            }
        }

        // if (DEBUG_LAYOUT) Slog.v(TAG, "Relayout " + win + ": viewVisibility=" + viewVisibility
        //         + " req=" + requestedWidth + "x" + requestedHeight + " " + win.mAttrs);

        Int32 privateFlags;
        win->mAttrs->GetPrivateFlags(&privateFlags);
        win->mEnforceSizeCompat =
                (privateFlags & IWindowManagerLayoutParams::PRIVATE_FLAG_COMPATIBLE_WINDOW) != 0;

        if ((attrChanges & IWindowManagerLayoutParams::ALPHA_CHANGED) != 0) {
            attrs->GetAlpha(&winAnimator->mAlpha);
        }

        win->mAttrs->GetFlags(&winFlags);
        Boolean scaledWindow =
            ((winFlags & IWindowManagerLayoutParams::FLAG_SCALED) != 0);

        if (scaledWindow) {
            // requested{Width|Height} Surface's physical size
            // attrs.{width|height} Size on screen
            Int32 width, height;
            IViewGroupLayoutParams::Probe(attrs)->GetWidth(&width);
            IViewGroupLayoutParams::Probe(attrs)->GetHeight(&height);
            win->mHScale = (width  != requestedWidth)  ?
                    (width  / (Float)requestedWidth) : 1.0f;
            win->mVScale = (height != requestedHeight) ?
                    (height / (Float)requestedHeight) : 1.0f;
        }
        else {
            win->mHScale = win->mVScale = 1;
        }

        Boolean imMayMove = (flagChanges & (
                IWindowManagerLayoutParams::FLAG_ALT_FOCUSABLE_IM |
                IWindowManagerLayoutParams::FLAG_NOT_FOCUSABLE)) != 0;

        Boolean isDefaultDisplay;
        win->IsDefaultDisplay(&isDefaultDisplay);
        Boolean focusMayChange = isDefaultDisplay && (win->mViewVisibility != viewVisibility
                || ((flagChanges & IWindowManagerLayoutParams::FLAG_NOT_FOCUSABLE) != 0)
                || (!win->mRelayoutCalled));

        Boolean wallpaperMayMove = win->mViewVisibility != viewVisibility
                && (winFlags & IWindowManagerLayoutParams::FLAG_SHOW_WALLPAPER) != 0;

        win->mRelayoutCalled = TRUE;
        Int32 oldVisibility = win->mViewVisibility;
        win->mViewVisibility = viewVisibility;
        // if (DEBUG_SCREEN_ON) {
        //     RuntimeException stack = new RuntimeException();
        //     stack.fillInStackTrace();
        //     Slog.i(TAG, "Relayout " + win + ": oldVis=" + oldVisibility
        //             + " newVis=" + viewVisibility, stack);
        // }
        if (viewVisibility == IView::VISIBLE &&
                (win->mAppToken == NULL || !win->mAppToken->mClientHidden)) {
            Boolean isVisible;
            win->IsVisibleLw(&isVisible);
            toBeDisplayed = !isVisible;
            if (win->mExiting) {
                winAnimator->CancelExitAnimationForNextAnimationLocked();
                win->mExiting = FALSE;
            }
            if (win->mDestroying) {
                win->mDestroying = FALSE;
                mDestroySurface.Remove(win);
            }
            if (oldVisibility == IView::GONE) {
                winAnimator->mEnterAnimationPending = TRUE;
            }
            if (toBeDisplayed) {
                if (win->IsDrawnLw() && OkToDisplay()) {
                    winAnimator->ApplyEnterAnimationLocked();
                }
                if ((winFlags & IWindowManagerLayoutParams::FLAG_TURN_SCREEN_ON) != 0) {
                    // if (DEBUG_VISIBILITY) Slog.v(TAG,
                    //         "Relayout window turning screen on: " + win);
                    win->mTurnOnScreen = TRUE;
                }
                if (win->IsConfigChanged()) {
                    // if (DEBUG_CONFIGURATION) Slog.i(TAG, "Window " + win
                    //         + " visible with new config: " + mCurConfiguration);
                    inConfig->SetTo(mCurConfiguration);
                }
            }
            if ((attrChanges & IWindowManagerLayoutParams::FORMAT_CHANGED) != 0) {
                // To change the format, we need to re-build the surface.
                winAnimator->DestroySurfaceLocked();
                toBeDisplayed = TRUE;
                surfaceChanged = TRUE;
            }
            // try {
            if (!win->mHasSurface) {
                surfaceChanged = TRUE;
            }
            AutoPtr<ISurfaceControl> surfaceControl = winAnimator->CreateSurfaceLocked();
            if (surfaceControl != NULL) {
                ECode ec = inSurface->CopyFrom(surfaceControl);
                if (FAILED(ec)) {
                    mInputMonitor->UpdateInputWindowsLw(TRUE /*force*/);
                    Binder::RestoreCallingIdentity(origId);
                    return 0;
                }
                // if (SHOW_TRANSACTIONS) Slog.i(TAG,
                //         "  OUT SURFACE " + outSurface + ": copied");
            }
            else {
                // For some reason there isn't a surface.  Clear the
                // caller's object so they see the same state.
                inSurface->ReleaseSurface();
            }
            // catch (Exception e) {
            //     mInputMonitor.updateInputWindowsLw(true /*force*/);

            //     Slog.w(TAG, "Exception thrown when creating surface for client "
            //              + client + " (" + win.mAttrs.getTitle() + ")",
            //              e);
            //     Binder::RestoreCallingIdentity(origId);
            //     return 0;
            // }
            if (toBeDisplayed) {
                focusMayChange = isDefaultDisplay;
            }
            Int32 winType;
            win->mAttrs->GetType(&winType);
            if (winType == IWindowManagerLayoutParams::TYPE_INPUT_METHOD
                    && mInputMethodWindow == NULL) {
                mInputMethodWindow = win;
                imMayMove = TRUE;
            }
            if (winType == IWindowManagerLayoutParams::TYPE_INPUT_METHOD
                    && win->mAppToken != NULL
                    && win->mAppToken->mStartingWindow != NULL) {
                // Special handling of starting window over the base
                // window of the app: propagate lock screen flags to it,
                // to provide the correct semantics while starting.
                Int32 mask =
                    IWindowManagerLayoutParams::FLAG_SHOW_WHEN_LOCKED
                    | IWindowManagerLayoutParams::FLAG_DISMISS_KEYGUARD
                    | IWindowManagerLayoutParams::FLAG_ALLOW_LOCK_WHILE_SCREEN_ON;
                AutoPtr<IWindowManagerLayoutParams> sa
                        = win->mAppToken->mStartingWindow->mAttrs;
                Int32 swFlags;
                sa->GetFlags(&swFlags);
                swFlags = (swFlags &~ mask) | (winFlags & mask);
                sa->SetFlags(swFlags);
            }
        }
        else {
            winAnimator->mEnterAnimationPending = FALSE;
            if (winAnimator->mSurfaceControl != NULL) {
                // if (DEBUG_VISIBILITY) Slog.i(TAG, "Relayout invis " + win
                //         + ": mExiting=" + win.mExiting);
                // If we are not currently running the exit animation, we
                // need to see about starting one.
                if (!win->mExiting) {
                    surfaceChanged = TRUE;
                    // Try starting an animation; if there isn't one, we
                    // can destroy the surface right away.
                    Int32 transit = IWindowManagerPolicy::TRANSIT_EXIT;
                    Int32 type1;
                    AutoPtr<IWindowManagerLayoutParams> winAttrs;
                    win->GetAttrs((IWindowManagerLayoutParams**)&winAttrs);
                    winAttrs->GetType(&type1);
                    if (type1 == IWindowManagerLayoutParams::TYPE_APPLICATION_STARTING) {
                        transit = IWindowManagerPolicy::TRANSIT_PREVIEW_DONE;
                    }
                    if (win->IsWinVisibleLw() &&
                            winAnimator->ApplyAnimationLocked(transit, FALSE)) {
                        focusMayChange = isDefaultDisplay;
                        win->mExiting = TRUE;
                    }
                    else if (win->mWinAnimator->IsAnimating()) {
                        // Currently in a hide animation... turn this into
                        // an exit.
                        win->mExiting = TRUE;
                    }
                    else if (win == mWallpaperTarget) {
                        // If the wallpaper is currently behind this
                        // window, we need to change both of them inside
                        // of a transaction to avoid artifacts.
                        win->mExiting = TRUE;
                        win->mWinAnimator->mAnimating = TRUE;
                    }
                    else {
                        if (mInputMethodWindow == win) {
                            mInputMethodWindow = NULL;
                        }
                        winAnimator->DestroySurfaceLocked();
                    }
                    //TODO (multidisplay): Magnification is supported only for the default
                    if (mAccessibilityController != NULL
                            && win->GetDisplayId() == IDisplay::DEFAULT_DISPLAY) {
                        mAccessibilityController->OnWindowTransitionLocked(win, transit);
                    }
                }
            }

            inSurface->ReleaseSurface();
            // if (DEBUG_VISIBILITY) Slog.i(TAG, "Releasing surface in: " + win);
        }

        if (focusMayChange) {
            // System.out.println("Focus may change: " + win.mAttrs.getTitle());
            if (UpdateFocusedWindowLocked(UPDATE_FOCUS_WILL_PLACE_SURFACES,
                    FALSE /*updateInputWindows*/)) {
                imMayMove = FALSE;
            }
            // System.out.println("Relayout " + win + ": focus=" + mCurrentFocus);
        }

        // updateFocusedWindowLocked() already assigned layers so we only need to
        // reassign them at this point if the IM window state gets shuffled
        if (imMayMove && (MoveInputMethodWindowsIfNeededLocked(FALSE) || toBeDisplayed)) {
            // Little hack here -- we -should- be able to rely on the
            // function to return true if the IME has moved and needs
            // its layer recomputed.  However, if the IME was hidden
            // and isn't actually moved in the list, its layer may be
            // out of data so we make sure to recompute it.
            AssignLayersLocked(win->GetWindowList());
        }

        if (wallpaperMayMove) {
            GetDefaultDisplayContentLocked()->mPendingLayoutChanges |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_WALLPAPER;
        }

        AutoPtr<DisplayContent> displayContent = win->GetDisplayContent();
        if (displayContent != NULL) {
            displayContent->mLayoutNeeded = TRUE;
        }
        win->mGivenInsetsPending = (flags & IWindowManagerGlobal::RELAYOUT_INSETS_PENDING) != 0;
        configChanged = UpdateOrientationFromAppTokensLocked(FALSE);
        PerformLayoutAndPlaceSurfacesLocked();
        if (toBeDisplayed && win->mIsWallpaper) {
            AutoPtr<IDisplayInfo> displayInfo = GetDefaultDisplayInfoLocked();
            Int32 width, height;
            displayInfo->GetLogicalWidth(&width);
            displayInfo->GetLogicalHeight(&height);
            UpdateWallpaperOffsetLocked(win, width, height, FALSE);
        }
        if (win->mAppToken != NULL) {
            win->mAppToken->UpdateReportedVisibilityLocked();
        }
        inFrame->Set(win->mCompatFrame);
        inOverscanInsets->Set(win->mOverscanInsets);
        inContentInsets->Set(win->mContentInsets);
        inVisibleInsets->Set(win->mVisibleInsets);
        inStableInsets->Set(win->mStableInsets);
        // if (localLOGV) Slog.v(
        //     TAG, "Relayout given client " + client.asBinder()
        //     + ", requestedWidth=" + requestedWidth
        //     + ", requestedHeight=" + requestedHeight
        //     + ", viewVisibility=" + viewVisibility
        //     + "\nRelayout returning frame=" + outFrame
        //     + ", surface=" + outSurface);

        // if (localLOGV || DEBUG_FOCUS) Slog.v(
        //     TAG, "Relayout of " + win + ": focusMayChange=" + focusMayChange);

        inTouchMode = mInTouchMode;
        animating = mAnimator->mAnimating && win->mWinAnimator->IsAnimating();;
        List< AutoPtr<WindowState> >::Iterator it = Find(mRelayoutWhileAnimating, win);
        if (animating && it == mRelayoutWhileAnimating.End()) {
            mRelayoutWhileAnimating.PushBack(win);
        }

        mInputMonitor->UpdateInputWindowsLw(TRUE /*force*/);

        // if (DEBUG_LAYOUT) {
        //     Slog.v(TAG, "Relayout complete " + win + ": outFrame=" + outFrame.toShortString());
        // }
    }

    if (configChanged) {
        SendNewConfiguration();
    }

    Binder::RestoreCallingIdentity(origId);

    *outFrame = inFrame;
    REFCOUNT_ADD(*outFrame);
    *outContentInsets = inContentInsets;
    REFCOUNT_ADD(*outContentInsets);
    *outVisibleInsets = inVisibleInsets;
    REFCOUNT_ADD(*outVisibleInsets);
    *outStableInsets = inStableInsets;
    REFCOUNT_ADD(*outStableInsets)
    *outConfig = inConfig;
    REFCOUNT_ADD(*outConfig);
    *outSurface = inSurface;
    REFCOUNT_ADD(*outSurface);

    return (inTouchMode ? IWindowManagerGlobal::RELAYOUT_RES_IN_TOUCH_MODE : 0)
            | (toBeDisplayed ? IWindowManagerGlobal::RELAYOUT_RES_FIRST_TIME : 0)
            | (surfaceChanged ? IWindowManagerGlobal::RELAYOUT_RES_SURFACE_CHANGED : 0)
            | (animating ? IWindowManagerGlobal::RELAYOUT_RES_ANIMATING : 0);
}

void CWindowManagerService::PerformDeferredDestroyWindow(
    /* [in] */ CSession* session,
    /* [in] */ IIWindow* client)
{
    Int64 origId = Binder::ClearCallingIdentity();

    // try {
    synchronized (mWindowMapLock) {
        AutoPtr<WindowState> win;
        WindowForClientLocked(session, client, FALSE, (WindowState**)&win);
        if (win == NULL) {
            Binder::RestoreCallingIdentity(origId);
            return;
        }
        win->mWinAnimator->DestroyDeferredSurfaceLocked();
        // } finally {
        Binder::RestoreCallingIdentity(origId);
        // }
    }
}

Boolean CWindowManagerService::OutOfMemoryWindow(
    /* [in] */ CSession* session,
    /* [in] */ IIWindow* client)
{
    Int64 origId = Binder::ClearCallingIdentity();

    // try {
    Boolean result;
    synchronized (mWindowMapLock) {
        AutoPtr<WindowState> win;
        WindowForClientLocked(session, client, FALSE, (WindowState**)&win);
        if (win == NULL) {
            Binder::RestoreCallingIdentity(origId);
            return FALSE;
        }
        Binder::RestoreCallingIdentity(origId);
        result = ReclaimSomeSurfaceMemoryLocked(win->mWinAnimator, String("from-client"), FALSE);
    }
    return result;
    // } finally {
    //     Binder.restoreCallingIdentity(origId);
    // }
}

void CWindowManagerService::FinishDrawingWindow(
    /* [in] */ CSession* session,
    /* [in] */ IIWindow* client)
{
    Int64 origId = Binder::ClearCallingIdentity();
    synchronized (mWindowMapLock) {
        AutoPtr<WindowState> win;
        WindowForClientLocked(session, client, FALSE, (WindowState**)&win);
        if (win != NULL && win->mWinAnimator->FinishDrawingLocked()) {
            Int32 flags;
            win->mAttrs->GetFlags(&flags);
            if ((flags & IWindowManagerLayoutParams::FLAG_SHOW_WALLPAPER) != 0) {
                AdjustWallpaperWindowsLocked();
            }
            win->mDisplayContent->mLayoutNeeded = TRUE;
            PerformLayoutAndPlaceSurfacesLocked();
        }

        Binder::RestoreCallingIdentity(origId);
    }
}

Boolean CWindowManagerService::ApplyAnimationLocked(
    /* [in] */ AppWindowToken* atoken,
    /* [in] */ IWindowManagerLayoutParams* lp,
    /* [in] */ Int32 transit,
    /* [in] */ Boolean enter,
    /* [in] */ Boolean isVoiceInteraction)
{
    // Only apply an animation if the display isn't frozen.  If it is
    // frozen, there is no reason to animate and it can cause strange
    // artifacts when we unfreeze the display if some different animation
    // is running.
    if (OkToDisplay()) {
        AutoPtr<IDisplayInfo> displayInfo = GetDefaultDisplayInfoLocked();
        Int32 width, height;
        displayInfo->GetAppWidth(&width);
        displayInfo->GetAppHeight(&height);
        if (DEBUG_APP_TRANSITIONS || DEBUG_ANIM) {
            Slogger::V(TAG, "applyAnimation: atoken=%p", atoken);
        }

        // Determine the visible rect to calculate the thumbnail clip
        AutoPtr<WindowState> win = atoken->FindMainWindow();
        AutoPtr<IRect> containingFrame;
        CRect::New(0, 0, width, height, (IRect**)&containingFrame);
        AutoPtr<IRect> contentInsets;
        CRect::New((IRect**)&contentInsets);
        Boolean isFullScreen = TRUE;
        if (win != NULL) {
            if (win->mContainingFrame != NULL) {
                containingFrame->Set(win->mContainingFrame);
            }
            if (win->mContentInsets != NULL) {
                contentInsets->Set(win->mContentInsets);
            }
            Int32 flags;
            isFullScreen =
                    ((win->mSystemUiVisibility & SYSTEM_UI_FLAGS_LAYOUT_STABLE_FULLSCREEN) ==
                            SYSTEM_UI_FLAGS_LAYOUT_STABLE_FULLSCREEN) ||
                            (win->mAttrs->GetFlags(&flags), (flags & IWindowManagerLayoutParams::FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS) != 0);
        }

        if (atoken->mLaunchTaskBehind) {
            // Differentiate the two animations. This one which is briefly on the screen
            // gets the !enter animation, and the other activity which remains on the
            // screen gets the enter animation. Both appear in the mOpeningApps set.
            enter = FALSE;
        }
        Int32 orientation;
        mCurConfiguration->GetOrientation(&orientation);
        AutoPtr<IAnimation> a = mAppTransition->LoadAnimation(lp, transit, enter, width, height,
                orientation, containingFrame, contentInsets, isFullScreen, isVoiceInteraction);
        if (a != NULL) {
            if (DEBUG_ANIM) {
                // RuntimeException e = null;
                // if (!HIDE_STACK_CRAWLS) {
                //     e = new RuntimeException();
                //     e.fillInStackTrace();
                // }
                Slogger::V(TAG, "Loaded animation %p for %p, Runtime Exception!", a.Get(), atoken);
            }
            atoken->mAppAnimator->SetAnimation(a, width, height);
        }
    }
    else {
        atoken->mAppAnimator->ClearAnimation();
    }

    return atoken->mAppAnimator->mAnimation != NULL;
}

// -------------------------------------------------------------
// Application Window Tokens
// -------------------------------------------------------------

void CWindowManagerService::ValidateAppTokens(
    /* [in] */ Int32 stackId,
    /* [in] */ IArrayList* tasks) //List<TaskGroup>
{
    synchronized (mWindowMapLock) {
        Int32 size;
        tasks->GetSize(&size);
        Int32 t = size - 1;
        if (t < 0) {
            Slogger::W(TAG, "validateAppTokens: empty task list");
            return;
        }

        AutoPtr<IInterface> obj;
        tasks->Get(0, (IInterface**)&obj);
        AutoPtr<TaskGroup> task = (TaskGroup*)IObject::Probe(obj);
        Int32 taskId = task->mTaskId;
        obj = NULL;
        mTaskIdToTask->Get(taskId, (IInterface**)&obj);
        AutoPtr<Task> targetTask = (Task*)IObject::Probe(obj);

        AutoPtr<DisplayContent> displayContent = targetTask->GetDisplayContent();
        if (displayContent == NULL) {
            Slogger::W(TAG, "validateAppTokens: no Display for taskId=%d", taskId);
            return;
        }

        obj = NULL;
        mStackIdToStack->Get(stackId, (IInterface**)&obj);
        AutoPtr<TaskStack> taskStack = (TaskStack*)IObject::Probe(obj);
        AutoPtr< List<AutoPtr<Task> > > localTasks = taskStack->GetTasks();

        List<AutoPtr<Task> >::ReverseIterator taskRit = localTasks->RBegin();
        for (; taskRit != localTasks->REnd() && t >= 0; ++taskRit, --t) {
            AppTokenList localTokens = (*taskRit)->mAppTokens;

            obj = NULL;
            tasks->Get(t, (IInterface**)&obj);
            task = (TaskGroup*)IObject::Probe(obj);

            List<AutoPtr<IApplicationToken> > tokens = task->mTokens;

            AutoPtr<DisplayContent> lastDisplayContent = displayContent;
            obj = NULL;
            mTaskIdToTask->Get(taskId, (IInterface**)&obj);
            targetTask = (Task*)IObject::Probe(obj);
            displayContent = targetTask->GetDisplayContent();
            if (displayContent.Get() != lastDisplayContent.Get()) {
                Slogger::W(TAG, "validateAppTokens: displayContent changed in TaskGroup list!");
                return;
            }

            AppTokenList::ReverseIterator tokenRit = localTokens.RBegin();
            List<AutoPtr<IApplicationToken> >::ReverseIterator appTokenRit = tokens.RBegin();
            while (taskRit != localTasks->REnd() && appTokenRit!= tokens.REnd()) {
                AutoPtr<AppWindowToken> atoken = *tokenRit;
                if (atoken->mRemoved) {
                    ++tokenRit;
                    continue;
                }
                if (IBinder::Probe(*appTokenRit) != atoken->mToken) {
                    break;
                }
                ++tokenRit;
                ++appTokenRit;
            }

            if (tokenRit != localTokens.REnd() || appTokenRit != tokens.REnd()) {
                break;
            }
        }

        if (taskRit != localTasks->REnd() || t >= 0) {
            Slogger::W(TAG, "validateAppTokens: Mismatch! ActivityManager=%s", TO_CSTR(tasks));
            Slogger::W(TAG, "validateAppTokens: Mismatch! WindowManager=");// + localTasks);
            Slogger::W(TAG, "validateAppTokens: Mismatch! Callers=");// + Debug.getCallers(4));
        }
    }
}

void CWindowManagerService::ValidateStackOrder(
    /* [in] */ ArrayOf<IInteger32*>* remoteStackIds)
{
    // TODO:
}

Boolean CWindowManagerService::CheckCallingPermission(
    /* [in] */ const String& permission,
    /* [in] */ const String& func)
{
    // Quick check: if the calling permission is me, it's all okay.
    if (Binder::GetCallingPid() == Process::MyPid()) {
      return TRUE;
    }

    Int32 perm;
    mContext->CheckCallingPermission(permission, &perm);
    if (perm == IPackageManager::PERMISSION_GRANTED) {
      return TRUE;
    }

    Slogger::W(TAG, "Permission Denial: %s from pid=%d, uid=%d requires %s",
            func.string(), Binder::GetCallingPid(), Binder::GetCallingUid(), permission.string());
    return FALSE;
}

Boolean CWindowManagerService::OkToDisplay()
{
    Boolean isON;
    mPolicy->IsScreenOn(&isON);
    return !mDisplayFrozen && mDisplayEnabled && isON;
}

AutoPtr<AppWindowToken> CWindowManagerService::FindAppWindowToken(
    /* [in] */ IBinder* token)
{
    HashMap<AutoPtr<IBinder>, AutoPtr<WindowToken>  >::Iterator
        wtit = mTokenMap.Find(AutoPtr<IBinder>(token));
    if (wtit != mTokenMap.End() && wtit->mSecond != NULL) {
        return wtit->mSecond->mAppWindowToken;
    }
    else {
        return NULL;
    }
}

ECode CWindowManagerService::AddWindowToken(
    /* [in] */ IBinder* token,
    /* [in] */ Int32 type)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("AddWindowToken()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized (mWindowMapLock) {
        AutoPtr<WindowToken> wtoken;
        HashMap<AutoPtr<IBinder>, AutoPtr<WindowToken> >::Iterator tokenIt = mTokenMap.Find(token);
        if (tokenIt != mTokenMap.End()) {
            wtoken = tokenIt->mSecond;
        }

        if (wtoken != NULL) {
            Slogger::W(TAG, "Attempted to add existing input method token: %p", token);
            return NOERROR;
        }
        wtoken = new WindowToken(this, token, type, TRUE);
        mTokenMap[token] = wtoken;
        if (type == IWindowManagerLayoutParams::TYPE_WALLPAPER) {
            mWallpaperTokens.PushBack(wtoken);
        }
    }

    return NOERROR;
}

ECode CWindowManagerService::RemoveWindowToken(
    /* [in] */ IBinder* token)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("RemoveWindowToken()"))) {
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    Int64 origId = Binder::ClearCallingIdentity();
    synchronized (mWindowMapLock) {
        AutoPtr<DisplayContent> displayContent;
        AutoPtr<WindowToken> wtoken;
        HashMap<AutoPtr<IBinder>, AutoPtr<WindowToken> >::Iterator tokenIt
                = mTokenMap.Find(token);
        if (tokenIt != mTokenMap.End()) {
            wtoken = tokenIt->mSecond;
            mTokenMap.Erase(tokenIt);
        }
        if (wtoken != NULL) {
            Boolean delayed = FALSE;
            if (!wtoken->mHidden) {
                Boolean changed = FALSE;
                List< AutoPtr<WindowState> >::Iterator it = wtoken->mWindows.Begin();
                for (; it != wtoken->mWindows.End(); ++it) {
                    AutoPtr<WindowState> win = *it;
                    displayContent = win->GetDisplayContent();

                    if (win->mWinAnimator->IsAnimating()) {
                        delayed = TRUE;
                    }

                    if (win->IsVisibleNow()) {
                        win->mWinAnimator->ApplyAnimationLocked(
                                IWindowManagerPolicy::TRANSIT_EXIT, FALSE);
                        //TODO (multidisplay): Magnification is supported only for the default
                        Boolean isDefaultDisplay;
                        if (mAccessibilityController != NULL && (win->IsDefaultDisplay(&isDefaultDisplay), isDefaultDisplay)) {
                            mAccessibilityController->OnWindowTransitionLocked(win,
                                    IWindowManagerPolicy::TRANSIT_EXIT);
                        }
                        changed = TRUE;
                        if (displayContent != NULL) {
                            displayContent->mLayoutNeeded = TRUE;
                        }
                    }
                }

                wtoken->mHidden = TRUE;

                if (changed) {
                    PerformLayoutAndPlaceSurfacesLocked();
                    UpdateFocusedWindowLocked(UPDATE_FOCUS_NORMAL,
                            FALSE /*updateInputWindows*/);
                }

                if (delayed) {
                    if (displayContent != NULL) {
                        displayContent->mExitingTokens.PushBack(wtoken);
                    }
                }
                else if (wtoken->mWindowType == IWindowManagerLayoutParams::TYPE_WALLPAPER) {
                    mWallpaperTokens.Remove(wtoken);
                }
            }

            mInputMonitor->UpdateInputWindowsLw(TRUE /*force*/);
        }
        else {
            Slogger::W(TAG, "Attempted to remove non-existing token: %p", token);
        }

        Binder::RestoreCallingIdentity(origId);
    }
    return NOERROR;
}

ECode CWindowManagerService::CreateTask(
    /* [in] */ Int32 taskId,
    /* [in] */ Int32 stackId,
    /* [in] */ Int32 userId,
    /* [in] */ AppWindowToken* atoken,
    /* [out] */ Task** _task)
{
    VALIDATE_NOT_NULL(_task)
    *_task = NULL;

    if (DEBUG_STACK) Slogger::I(TAG, "createTask: taskId=%d stackId=%d atoken=%p", taskId, stackId, atoken);
    AutoPtr<IInterface> value;
    mStackIdToStack->Get(stackId, (IInterface**)&value);
    AutoPtr<TaskStack> stack = (TaskStack*)(IObject*)value.Get();
    if (stack == NULL) {
        Slogger::E(TAG, "addAppToken: invalid stackId=%d", stackId);
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
        // throw new IllegalArgumentException("addAppToken: invalid stackId=" + stackId);
    }
    // EventLog.writeEvent(EventLogTags.WM_TASK_CREATED, taskId, stackId);
    AutoPtr<Task> task = new Task(atoken, stack, userId);
    mTaskIdToTask->Put(taskId, (IObject*)task);
    stack->AddTask(task, !atoken->mLaunchTaskBehind /* toTop */);
    *_task = task;
    REFCOUNT_ADD(*_task)
    return NOERROR;
}

ECode CWindowManagerService::AddAppToken(
    /* [in] */ Int32 addPos,
    /* [in] */ IApplicationToken* token,
    /* [in] */ Int32 taskId,
    /* [in] */ Int32 stackId,
    /* [in] */ Int32 requestedOrientation,
    /* [in] */ Boolean fullscreen,
    /* [in] */ Boolean showWhenLocked,
    /* [in] */ Int32 userId,
    /* [in] */ Int32 configChanges,
    /* [in] */ Boolean voiceInteraction,
    /* [in] */ Boolean launchTaskBehind)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("AddAppToken()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    // Get the dispatching timeout here while we are not holding any locks so that it
    // can be cached by the AppWindowToken.  The timeout value is used later by the
    // input dispatcher in code that does hold locks.  If we did not cache the value
    // here we would run the chance of introducing a deadlock between the window manager
    // (which holds locks while updating the input dispatcher state) and the activity manager
    // (which holds locks while querying the application token).
    Int64 inputDispatchingTimeoutNanos;
    // try {
    Int64 timeout;
    ECode ec = token->GetKeyDispatchingTimeout(&timeout);
    if (FAILED(ec)) {
        Slogger::W(TAG, "Could not get dispatching timeout.0x%08x", ec);
        inputDispatchingTimeoutNanos = DEFAULT_INPUT_DISPATCHING_TIMEOUT_NANOS;
    }
    else {
        inputDispatchingTimeoutNanos = timeout * 1000000ll;
    }
    // } catch (RemoteException ex) {
    //     Slog.w(TAG, "Could not get dispatching timeout.", ex);
    //     inputDispatchingTimeoutNanos = DEFAULT_INPUT_DISPATCHING_TIMEOUT_NANOS;
    // }

    synchronized (mWindowMapLock) {
        AutoPtr<AppWindowToken> atoken = FindAppWindowToken(IBinder::Probe(token));
        if (atoken != NULL) {
            Slogger::W(TAG, "Attempted to add existing app token: %p", token);
            return NOERROR;
        }

        atoken = new AppWindowToken(this, token, voiceInteraction);
        atoken->Init();

        atoken->mInputDispatchingTimeoutNanos = inputDispatchingTimeoutNanos;
        atoken->mGroupId = taskId;
        atoken->mAppFullscreen = fullscreen;
        atoken->mShowWhenLocked = showWhenLocked;
        atoken->mRequestedOrientation = requestedOrientation;
        atoken->mLayoutConfigChanges = (configChanges &
                (IActivityInfo::CONFIG_SCREEN_SIZE | IActivityInfo::CONFIG_ORIENTATION)) != 0;
        atoken->mLaunchTaskBehind = launchTaskBehind;
        if (DEBUG_TOKEN_MOVEMENT || DEBUG_ADD_REMOVE) {
            Slogger::V(TAG, "addAppToken: %p to stack=%d task= %d at %d", atoken.Get(), stackId, taskId, addPos);
        }

        AutoPtr<IInterface> value;
        mTaskIdToTask->Get(taskId, (IInterface**)&value);
        AutoPtr<Task> task = (Task*)(IObject*)value.Get();
        if (task == NULL) {
            CreateTask(taskId, stackId, userId, atoken, (Task**)&task);
        }
        else {
            task->AddAppToken(addPos, atoken);
        }

        mTokenMap[IBinder::Probe(token)] = atoken;

        // Application tokens start out hidden.
        atoken->mHidden = TRUE;
        atoken->mHiddenRequested = TRUE;
    }

    return NOERROR;
}

ECode CWindowManagerService::SetAppGroupId(
    /* [in] */ IBinder* token,
    /* [in] */ Int32 groupId)
{
    if (!CheckCallingPermission(
        Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
        String("SetAppStartingIcon()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized (mWindowMapLock) {
        AutoPtr<AppWindowToken> atoken = FindAppWindowToken(token);
        if (atoken == NULL) {
            Slogger::W(TAG, "Attempted to set group id of non-existing app token: %p", token);
            return NOERROR;
        }
        AutoPtr<IInterface> value;
        mTaskIdToTask->Get(atoken->mGroupId, (IInterface**)&value);
        AutoPtr<Task> oldTask = (Task*)(IObject*)value.Get();
        oldTask->RemoveAppToken(atoken);

        atoken->mGroupId = groupId;
        value = NULL;
        mTaskIdToTask->Get(groupId, (IInterface**)&value);
        AutoPtr<Task> newTask = (Task*)(IObject*)value.Get();
        if (newTask == NULL) {
            CreateTask(groupId, oldTask->mStack->mStackId, oldTask->mUserId, atoken, (Task**)&newTask);
        }
        else {
            newTask->mAppTokens.PushBack(atoken);
        }
    }
    return NOERROR;
}

Int32 CWindowManagerService::GetOrientationFromWindowsLocked()
{
    if (mDisplayFrozen || mOpeningApps.Begin() != mOpeningApps.End()
            || mClosingApps.Begin() != mClosingApps.End()) {
        // If the display is frozen, some activities may be in the middle
        // of restarting, and thus have removed their old window.  If the
        // window has the flag to hide the lock screen, then the lock screen
        // can re-appear and inflict its own orientation on us.  Keep the
        // orientation stable until this all settles down.
        return mLastWindowForcedOrientation;
    }

    // TODO(multidisplay): Change to the correct display.
    AutoPtr<WindowList> windows = GetDefaultWindowListLocked();
    WindowList::ReverseIterator rit = windows->RBegin();
    while (rit != windows->REnd()) {
        AutoPtr<WindowState> wtoken = *rit;
        ++rit;
        if (wtoken->mAppToken != NULL) {
            // We hit an application window. so the orientation will be determined by the
            // app window. No point in continuing further.
            return (mLastWindowForcedOrientation = IActivityInfo::SCREEN_ORIENTATION_UNSPECIFIED);
        }
        Boolean isVisible;
        if ((wtoken->IsVisibleLw(&isVisible), !isVisible) || !wtoken->mPolicyVisibilityAfterAnim) {
            continue;
        }

        Int32 req;
        wtoken->mAttrs->GetScreenOrientation(&req);
        if((req == IActivityInfo::SCREEN_ORIENTATION_UNSPECIFIED) ||
                (req == IActivityInfo::SCREEN_ORIENTATION_BEHIND)){
            continue;
        }

        // if (DEBUG_ORIENTATION) Slog.v(TAG, win + " forcing orientation to " + req);
        return (mLastWindowForcedOrientation = req);
    }
    return (mLastWindowForcedOrientation = IActivityInfo::SCREEN_ORIENTATION_UNSPECIFIED);
}

Int32 CWindowManagerService::GetOrientationFromAppTokensLocked()
{
    Int32 lastOrientation = IActivityInfo::SCREEN_ORIENTATION_UNSPECIFIED;
    Boolean findingBehind = FALSE;
    Boolean lastFullscreen = FALSE;
    // TODO: Multi window.
    AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();
    AutoPtr< List<AutoPtr<Task> > > tasks = displayContent->GetTasks();
    List<AutoPtr<Task> >::ReverseIterator rit = tasks->RBegin();
    for (; rit != tasks->REnd(); ++rit) {
        AppTokenList tokens = (*rit)->mAppTokens;
        AppTokenList::ReverseIterator firstTokenRit = tokens.RBegin();
        for (; firstTokenRit != tokens.REnd(); ++firstTokenRit) {
            AutoPtr<AppWindowToken> atoken = *firstTokenRit;

            if (DEBUG_APP_ORIENTATION) Slogger::V(TAG, "Checking app orientation: %p", atoken.Get());

            // if we're about to tear down this window and not seek for
            // the behind activity, don't use it for orientation
            if (!findingBehind
                    && (!atoken->mHidden && atoken->mHiddenRequested)) {
                if (DEBUG_ORIENTATION) Slogger::V(TAG, "Skipping %p -- going to hide", atoken.Get());
                continue;
            }

            if (firstTokenRit == tokens.RBegin()) {
                // If we have hit a new Task, and the bottom
                // of the previous group didn't explicitly say to use
                // the orientation behind it, and the last app was
                // full screen, then we'll stick with the
                // user's orientation.
                if (lastOrientation != IActivityInfo::SCREEN_ORIENTATION_BEHIND
                        && lastFullscreen) {
                    if (DEBUG_ORIENTATION) {
                        Slogger::V(TAG, "Done at %p -- end of group, return %d", atoken.Get(), lastOrientation);
                    }
                    return lastOrientation;
                }
            }

            // We ignore any hidden applications on the top.
            if (atoken->mHiddenRequested || atoken->mWillBeHidden) {
                if (DEBUG_ORIENTATION) Slogger::V(TAG, "Skipping %p -- hidden on top", atoken.Get());
                continue;
            }

            if (tokens.Begin() == tokens.End()) {
                // Last token in this task.
                lastOrientation = atoken->mRequestedOrientation;
            }

            Int32 orientation = atoken->mRequestedOrientation;
            // If this application is fullscreen, and didn't explicitly say
            // to use the orientation behind it, then just take whatever
            // orientation it has and ignores whatever is under it.
            lastFullscreen = atoken->mAppFullscreen;
            if (lastFullscreen && orientation != IActivityInfo::SCREEN_ORIENTATION_BEHIND) {
                if (DEBUG_ORIENTATION) {
                    Slogger::V(TAG, "Done at %p -- full screen, return %d", atoken.Get(), orientation);
                }
                return orientation;
            }
            // If this application has requested an explicit orientation,
            // then use it.
            if (orientation != IActivityInfo::SCREEN_ORIENTATION_UNSPECIFIED
                    && orientation != IActivityInfo::SCREEN_ORIENTATION_BEHIND) {
                if (DEBUG_ORIENTATION) Slogger::V(TAG, "Done at %p -- explicitly set, return %d", atoken.Get(), orientation);
                return orientation;
            }
            findingBehind |= (orientation == IActivityInfo::SCREEN_ORIENTATION_BEHIND);
        }
    }
    if (DEBUG_ORIENTATION) Slogger::V(TAG, "No app is requesting an orientation");
    return IActivityInfo::SCREEN_ORIENTATION_UNSPECIFIED;
}

ECode CWindowManagerService::UpdateOrientationFromAppTokens(
    /* [in] */ IConfiguration* currentConfig,
    /* [in] */ IBinder* freezeThisOneIfNeeded,
    /* [out] */ IConfiguration** config)
{
    VALIDATE_NOT_NULL(config);

    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("UpdateOrientationFromAppTokens()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    Int64 ident = Binder::ClearCallingIdentity();

    synchronized (mWindowMapLock) {
        AutoPtr<IConfiguration> temp = UpdateOrientationFromAppTokensLocked(
                currentConfig, freezeThisOneIfNeeded);

        Binder::RestoreCallingIdentity(ident);
        *config = temp;
        REFCOUNT_ADD(*config);
    }
    return NOERROR;
}

AutoPtr<IConfiguration> CWindowManagerService::UpdateOrientationFromAppTokensLocked(
    /* [in] */ IConfiguration* currentConfig,
    /* [in] */ IBinder* freezeThisOneIfNeeded)
{
    AutoPtr<IConfiguration> config;

    if (UpdateOrientationFromAppTokensLocked(FALSE)) {
        if (freezeThisOneIfNeeded != NULL) {
            AutoPtr<AppWindowToken> atoken = FindAppWindowToken(freezeThisOneIfNeeded);
            if (atoken != NULL) {
                StartAppFreezingScreenLocked(atoken);
            }
        }
        config = ComputeNewConfigurationLocked();

    }
    else if (currentConfig != NULL) {
        // No obvious action we need to take, but if our current
        // state mismatches the activity manager's, update it,
        // disregarding font scale, which should remain set to
        // the value of the previous configuration.
        mTempConfiguration->SetToDefaults();
        Float fontScale;
        currentConfig->GetFontScale(&fontScale);
        mTempConfiguration->SetFontScale(fontScale);
        if (ComputeScreenConfigurationLocked(mTempConfiguration)) {
            Int32 res;
            currentConfig->Diff(mTempConfiguration, &res);
            if (res != 0) {
                mWaitingForConfig = TRUE;
                AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();
                displayContent->mLayoutNeeded = TRUE;
                AutoPtr< ArrayOf<Int32> > anim = ArrayOf<Int32>::Alloc(2);
                if (displayContent->IsDimming()) {
                    (*anim)[0] = (*anim)[1] = 0;
                }
                else {
                    mPolicy->SelectRotationAnimationLw(anim);
                }
                StartFreezingDisplayLocked(FALSE, (*anim)[0], (*anim)[1]);
                ASSERT_SUCCEEDED(CConfiguration::New(mTempConfiguration, (IConfiguration**)&config));
            }
        }
    }

    return config;
}

Boolean CWindowManagerService::UpdateOrientationFromAppTokensLocked(
    /* [in] */ Boolean inTransaction)
{
    Int64 ident = Binder::ClearCallingIdentity();

    //try {
    Int32 req = GetOrientationFromWindowsLocked();
    if (req == IActivityInfo::SCREEN_ORIENTATION_UNSPECIFIED) {
        req = GetOrientationFromAppTokensLocked();
    }

    if (req != mForcedAppOrientation) {
        mForcedAppOrientation = req;
        //send a message to Policy indicating orientation change to take
        //action like disabling/enabling sensors etc.,
        mPolicy->SetCurrentOrientationLw(req);
        if (UpdateRotationUncheckedLocked(inTransaction)) {
            // changed
            Binder::RestoreCallingIdentity(ident);
            return TRUE;
        }
    }

    Binder::RestoreCallingIdentity(ident);
    return FALSE;
}

ECode CWindowManagerService::SetNewConfiguration(
    /* [in] */ IConfiguration* config)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("SetNewConfiguration()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized (mWindowMapLock) {
        mCurConfiguration = NULL;
        CConfiguration::New(config, (IConfiguration**)&mCurConfiguration);
        if (mWaitingForConfig) {
            mWaitingForConfig = FALSE;
            AutoPtr<ICharSequence> cs;
            CString::New(String("new-config"), (ICharSequence**)&cs);
            mLastFinishedFreezeSource = IObject::Probe(cs);
        }
        PerformLayoutAndPlaceSurfacesLocked();
    }
    return NOERROR;
}

ECode CWindowManagerService::SetAppOrientation(
    /* [in] */ IApplicationToken* token,
    /* [in] */ Int32 requestedOrientation)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("SetAppOrientation()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized(mWindowMapLock) {
        AutoPtr<AppWindowToken> atoken = FindAppWindowToken(IBinder::Probe(token));
        if (atoken == NULL) {
            Slogger::W(TAG, "Attempted to set orientation of non-existing app token: %p", token);
            return NOERROR;
        }

        atoken->mRequestedOrientation = requestedOrientation;
    }

    return NOERROR;
}

ECode CWindowManagerService::GetAppOrientation(
    /* [in] */ IApplicationToken* token,
    /* [out] */ Int32* orientation)
{
    VALIDATE_NOT_NULL(orientation);

    synchronized(mWindowMapLock) {
        AutoPtr<AppWindowToken> wtoken = FindAppWindowToken(IBinder::Probe(token));
        if (wtoken == NULL) {
            *orientation = IActivityInfo::SCREEN_ORIENTATION_UNSPECIFIED;
            return NOERROR;
        }

        *orientation = wtoken->mRequestedOrientation;
    }
    return NOERROR;
}

void CWindowManagerService::SetFocusedStackLayer()
{
    mFocusedStackLayer = 0;
    if (mFocusedApp != NULL) {
        WindowList windows = mFocusedApp->mAllAppWindows;
        WindowList::ReverseIterator rit = windows.RBegin();
        for (; rit != windows.REnd(); ++rit) {
            AutoPtr<WindowState> win = *rit;
            Int32 animLayer = win->mWinAnimator->mAnimLayer;
            Boolean isVisible;
            if (win->mAttachedWindow == NULL && (win->IsVisibleLw(&isVisible), isVisible) &&
                    animLayer > mFocusedStackLayer) {
                mFocusedStackLayer = animLayer + LAYER_OFFSET_FOCUSED_STACK;
            }
        }
    }
    if (DEBUG_LAYERS) Slogger::V(TAG, "Setting FocusedStackFrame to layer=%d", mFocusedStackLayer);
    mFocusedStackFrame->SetLayer(mFocusedStackLayer);
}

void CWindowManagerService::SetFocusedStackFrame()
{
    AutoPtr<TaskStack> stack;
    if (mFocusedApp != NULL) {
        AutoPtr<IInterface> value;
        mTaskIdToTask->Get(mFocusedApp->mGroupId, (IInterface**)&value);
        AutoPtr<Task> task = (Task*)(IObject*)value.Get();
        stack = task->mStack;
        AutoPtr<DisplayContent> displayContent = task->GetDisplayContent();
        if (displayContent != NULL) {
            displayContent->SetTouchExcludeRegion(stack);
        }
    }
    else {
        stack = NULL;
    }
    if (SHOW_LIGHT_TRANSACTIONS) Slogger::I(TAG, ">>> OPEN TRANSACTION setFocusedStackFrame");
    AutoPtr<ISurfaceControlHelper> helper;
    CSurfaceControlHelper::AcquireSingleton((ISurfaceControlHelper**)&helper);
    helper->OpenTransaction();
    // try {
    if (stack == NULL) {
        mFocusedStackFrame->SetVisibility(FALSE);
    }
    else {
        mFocusedStackFrame->SetBounds(stack);
        Boolean multipleStacks = !stack->IsFullscreen();
        mFocusedStackFrame->SetVisibility(multipleStacks);
    }
    // } finally {
    //     SurfaceControl.closeTransaction();
    //     if (SHOW_LIGHT_TRANSACTIONS) Slog.i(TAG, ">>> CLOSE TRANSACTION setFocusedStackFrame");
    // }
    helper->CloseTransaction();
    if (SHOW_LIGHT_TRANSACTIONS) Slogger::I(TAG, ">>> CLOSE TRANSACTION setFocusedStackFrame");
}

ECode CWindowManagerService::SetFocusedApp(
    /* [in] */ IBinder* token,
    /* [in] */ Boolean moveFocusNow)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("SetFocusedApp()"))) {
        //throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized (mWindowMapLock) {
        AutoPtr<AppWindowToken> newFocus;
        if (token == NULL) {
            if (DEBUG_FOCUS_LIGHT) Slogger::V(TAG, "Clearing focused app, was %p", mFocusedApp.Get());
            newFocus = NULL;
        }
        else {
            newFocus = FindAppWindowToken(token);
            if (newFocus == NULL) {
                Slogger::W(TAG, "Attempted to set focus to non-existing app token: %p", token);
            }
            if (DEBUG_FOCUS_LIGHT)
                Slogger::V(TAG, "Set focused app to: %p old focus=%p moveFocusNow=%d"
                        , newFocus.Get(), mFocusedApp.Get(), moveFocusNow);
        }

        Boolean changed = mFocusedApp != newFocus;
        if (changed) {
            mFocusedApp = newFocus;
            mInputMonitor->SetFocusedAppLw(newFocus);
        }

        if (moveFocusNow && changed) {
            Int64 origId = Binder::ClearCallingIdentity();
            UpdateFocusedWindowLocked(UPDATE_FOCUS_NORMAL, TRUE /*updateInputWindows*/);
            Binder::RestoreCallingIdentity(origId);
        }
    }

    return NOERROR;
}

ECode CWindowManagerService::PrepareAppTransition(
    /* [in] */ Int32 transit,
    /* [in] */ Boolean alwaysKeepCurrent)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("PrepareAppTransition()"))) {
        //throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized(mWindowMapLock) {
        if (DEBUG_APP_TRANSITIONS) {
            Slogger::V(TAG, "Prepare app transition: transit=%d %p alwaysKeepCurrent=%d Callers="
                    , transit, mAppTransition.Get(), alwaysKeepCurrent/*, Debug.getCallers(3)*/);
        }
        if (!mAppTransition->IsTransitionSet() || mAppTransition->IsTransitionNone()) {
            mAppTransition->SetAppTransition(transit);
        }
        else if (!alwaysKeepCurrent) {
            if (transit == AppTransition::TRANSIT_TASK_OPEN
                    && mAppTransition->IsTransitionEqual(AppTransition::TRANSIT_TASK_CLOSE)) {
                // Opening a new task always supersedes a close for the anim.
                mAppTransition->SetAppTransition(transit);
            }
            else if (transit == AppTransition::TRANSIT_ACTIVITY_OPEN
                    && mAppTransition->IsTransitionEqual(AppTransition::TRANSIT_ACTIVITY_CLOSE)) {
                // Opening a new activity always supersedes a close for the anim.
                mAppTransition->SetAppTransition(transit);
            }
        }
        if (OkToDisplay()) {
            mAppTransition->Prepare();
            mStartingIconInTransition = FALSE;
            mSkipAppTransitionAnimation = FALSE;
            mH->RemoveMessages(H::APP_TRANSITION_TIMEOUT);
            Boolean result;
            mH->SendEmptyMessageDelayed(H::APP_TRANSITION_TIMEOUT, 5000, &result);
        }
    }

    return NOERROR;
}

ECode CWindowManagerService::GetPendingAppTransition(
    /* [out] */ Int32* transit)
{
    VALIDATE_NOT_NULL(transit)
    *transit = mAppTransition->GetAppTransition();
    return NOERROR;
}

ECode CWindowManagerService::OverridePendingAppTransition(
    /* [in] */ const String& packageName,
    /* [in] */ Int32 enterAnim,
    /* [in] */ Int32 exitAnim,
    /* [in] */ IIRemoteCallback* startedCallback)
{
    synchronized (mWindowMapLock) {
        mAppTransition->OverridePendingAppTransition(packageName, enterAnim, exitAnim,
                startedCallback);
    }
    return NOERROR;
}

ECode CWindowManagerService::OverridePendingAppTransitionScaleUp(
    /* [in] */ Int32 startX,
    /* [in] */ Int32 startY,
    /* [in] */ Int32 startWidth,
    /* [in] */ Int32 startHeight)
{
    synchronized (mWindowMapLock) {
        mAppTransition->OverridePendingAppTransitionScaleUp(startX, startY, startWidth, startHeight);
    }
    return NOERROR;
}

ECode CWindowManagerService::OverridePendingAppTransitionThumb(
    /* [in] */ IBitmap* srcThumb,
    /* [in] */ Int32 startX,
    /* [in] */ Int32 startY,
    /* [in] */ IIRemoteCallback* startedCallback,
    /* [in] */ Boolean scaleUp)
{
    synchronized (mWindowMapLock) {
        mAppTransition->OverridePendingAppTransitionThumb(srcThumb, startX, startY,
                startedCallback, scaleUp);
    }
    return NOERROR;
}

ECode CWindowManagerService::OverridePendingAppTransitionAspectScaledThumb(
    /* [in] */ IBitmap* srcThumb,
    /* [in] */ Int32 startX,
    /* [in] */ Int32 startY,
    /* [in] */ Int32 targetWidth,
    /* [in] */ Int32 targetHeight,
    /* [in] */ IIRemoteCallback* startedCallback,
    /* [in] */ Boolean scaleUp)
{
    synchronized (mWindowMapLock) {
        mAppTransition->OverridePendingAppTransitionAspectScaledThumb(srcThumb, startX,
                startY, targetWidth, targetHeight, startedCallback, scaleUp);
    }
    return NOERROR;
}

ECode CWindowManagerService::ExecuteAppTransition()
{
// Slogger::D(TAG, "==== File: %s, Line: %d ====", __FILE__, __LINE__);
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("ExecuteAppTransition()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized(mWindowMapLock) {
        if (DEBUG_APP_TRANSITIONS) {
            Slogger::W(TAG, "Execute app transition: %p", mAppTransition.Get()/*, new RuntimeException("here").fillInStackTrace()*/);
        }

        if (mAppTransition->IsTransitionSet()) {
            mAppTransition->SetReady();
            Int64 origId = Binder::ClearCallingIdentity();
            // try {
            PerformLayoutAndPlaceSurfacesLocked();
            // } finally {
            Binder::RestoreCallingIdentity(origId);
            // }
        }
    }

    return NOERROR;
}

ECode CWindowManagerService::SetAppStartingWindow(
    /* [in] */ IBinder* token,
    /* [in] */ const String& pkg,
    /* [in] */ Int32 theme,
    /* [in] */ ICompatibilityInfo* compatInfo,
    /* [in] */ ICharSequence* nonLocalizedLabel,
    /* [in] */ Int32 labelRes,
    /* [in] */ Int32 icon,
    /* [in] */ Int32 logo,
    /* [in] */ Int32 windowFlags,
    /* [in] */ IBinder* transferFrom,
    /* [in] */ Boolean createIfNeeded)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("SetAppStartingIcon()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    AutoLock lock(mWindowMapLock);

    if (DEBUG_STARTING_WINDOW) {
        Slogger::V(TAG, "setAppStartingWindow: token=%p pkg=%s transferFrom=%p",
            token, pkg.string(), transferFrom);
    }

    AutoPtr<AppWindowToken> wtoken = FindAppWindowToken(token);
    if (wtoken == NULL) {
        // Slog.w(TAG, "Attempted to set icon of non-existing app token: " + token);
        return NOERROR;
    }

    // If the display is frozen, we won't do anything until the
    // actual window is displayed so there is no reason to put in
    // the starting window.
    if (!OkToDisplay()) {
        return NOERROR;
    }

    if (wtoken->mStartingData != NULL) {
        return NOERROR;
    }

    if (transferFrom != NULL) {
        AutoPtr<AppWindowToken> ttoken = FindAppWindowToken(transferFrom);
        if (ttoken != NULL) {
            AutoPtr<WindowState> startingWindow = ttoken->mStartingWindow;
            if (startingWindow != NULL) {
                if (mStartingIconInTransition) {
                    // In this case, the starting icon has already
                    // been displayed, so start letting windows get
                    // shown immediately without any more transitions.
                    mSkipAppTransitionAnimation = TRUE;
                }
                if (DEBUG_STARTING_WINDOW) {
                    Slogger::V(TAG, "Moving existing starting %p from %p to %p",
                        startingWindow.Get(), ttoken.Get(), wtoken.Get());
                }
                Int64 origId = Binder::ClearCallingIdentity();

                // Transfer the starting window over to the new
                // token.
                wtoken->mStartingData = ttoken->mStartingData;
                wtoken->mStartingView = ttoken->mStartingView;
                wtoken->mStartingDisplayed = ttoken->mStartingDisplayed;
                wtoken->mStartingWindow = startingWindow;
                ttoken->mStartingData = NULL;
                ttoken->mStartingView = NULL;
                ttoken->mStartingWindow = NULL;
                ttoken->mStartingMoved = TRUE;
                startingWindow->mToken = wtoken;
                startingWindow->mRootToken = wtoken;
                startingWindow->mAppToken = wtoken;
                startingWindow->mWinAnimator->mAppAnimator = wtoken->mAppAnimator;

                if (DEBUG_WINDOW_MOVEMENT || DEBUG_ADD_REMOVE || DEBUG_STARTING_WINDOW) {
                    Slogger::V(TAG, "Removing starting window: %p", startingWindow.Get());
                }
                startingWindow->GetWindowList()->Remove(startingWindow);
                mWindowsChanged = TRUE;
                if (DEBUG_ADD_REMOVE) {
                    Slogger::V(TAG, "Removing starting %p from %p", startingWindow.Get(), ttoken.Get());
                }
                ttoken->mWindows.Remove(startingWindow);
                ttoken->mAllAppWindows.Remove(startingWindow);
                AddWindowToListInOrderLocked(startingWindow, TRUE);

                // Propagate other interesting state between the
                // tokens.  If the old token is displayed, we should
                // immediately force the new one to be displayed.  If
                // it is animating, we need to move that animation to
                // the new one.
                if (ttoken->mAllDrawn) {
                    wtoken->mAllDrawn = TRUE;
                    wtoken->mDeferClearAllDrawn = ttoken->mDeferClearAllDrawn;
                }
                if (ttoken->mFirstWindowDrawn) {
                    wtoken->mFirstWindowDrawn = TRUE;
                }
                if (!ttoken->mHidden) {
                    wtoken->mHidden = FALSE;
                    wtoken->mHiddenRequested = FALSE;
                    wtoken->mWillBeHidden = FALSE;
                }
                if (wtoken->mClientHidden != ttoken->mClientHidden) {
                    wtoken->mClientHidden = ttoken->mClientHidden;
                    wtoken->SendAppVisibilityToClients();
                }
                AutoPtr<AppWindowAnimator> tAppAnimator = ttoken->mAppAnimator;
                AutoPtr<AppWindowAnimator> wAppAnimator = wtoken->mAppAnimator;
                if (tAppAnimator->mAnimation != NULL) {
                    wAppAnimator->mAnimation = tAppAnimator->mAnimation;
                    wAppAnimator->mAnimating = tAppAnimator->mAnimating;
                    wAppAnimator->mAnimLayerAdjustment = tAppAnimator->mAnimLayerAdjustment;
                    tAppAnimator->mAnimation = NULL;
                    tAppAnimator->mAnimLayerAdjustment = 0;
                    wAppAnimator->UpdateLayers();
                    tAppAnimator->UpdateLayers();
                }

                UpdateFocusedWindowLocked(UPDATE_FOCUS_WILL_PLACE_SURFACES,
                        TRUE /*updateInputWindows*/);
                GetDefaultDisplayContentLocked()->mLayoutNeeded = TRUE;
                PerformLayoutAndPlaceSurfacesLocked();
                Binder::RestoreCallingIdentity(origId);
                return NOERROR;
            }
            else if (ttoken->mStartingData != NULL) {
                // The previous app was getting ready to show a
                // starting window, but hasn't yet done so.  Steal it!
                if (DEBUG_STARTING_WINDOW) {
                    Slogger::V(TAG, "Moving pending starting from %p to %p",
                        ttoken.Get(), wtoken.Get());
                }
                wtoken->mStartingData = ttoken->mStartingData;
                ttoken->mStartingData = NULL;
                ttoken->mStartingMoved = TRUE;

                AutoPtr<IMessage> msg;
                mH->ObtainMessage(H::ADD_STARTING, (IObject*)wtoken.Get(), (IMessage**)&msg);

                // Note: we really want to do sendMessageAtFrontOfQueue() because we
                // want to process the message ASAP, before any other queued
                // messages.
                Boolean result;
                return mH->SendMessageAtFrontOfQueue(msg, &result);
            }

            AutoPtr<AppWindowAnimator> tAppAnimator = ttoken->mAppAnimator;
            AutoPtr<AppWindowAnimator> wAppAnimator = wtoken->mAppAnimator;
            if (tAppAnimator->mThumbnail != NULL) {
                // The old token is animating with a thumbnail, transfer
                // that to the new token.
                if (wAppAnimator->mThumbnail != NULL) {
                    wAppAnimator->mThumbnail->Destroy();
                }
                wAppAnimator->mThumbnail = tAppAnimator->mThumbnail;
                wAppAnimator->mThumbnailX = tAppAnimator->mThumbnailX;
                wAppAnimator->mThumbnailY = tAppAnimator->mThumbnailY;
                wAppAnimator->mThumbnailLayer = tAppAnimator->mThumbnailLayer;
                wAppAnimator->mThumbnailAnimation = tAppAnimator->mThumbnailAnimation;
                tAppAnimator->mThumbnail = NULL;
            }
        }
    }

    // There is no existing starting window, and the caller doesn't
    // want us to create one, so that's it!
    if (!createIfNeeded) {
        return NOERROR;
    }

    // If this is a translucent or wallpaper window, then don't
    // show a starting window -- the current effect (a full-screen
    // opaque starting window that fades away to the real contents
    // when it is ready) does not work for this.
    if (DEBUG_STARTING_WINDOW) {
        Slogger::V(TAG, "Checking theme of starting window: 0x%d", theme);
    }
    if (theme != 0) {
        AutoPtr<ArrayOf<Int32> > attrIds = ArrayOf<Int32>::Alloc(
            const_cast<Int32 *>(Elastos::Droid::R::styleable::Window),
            ArraySize(Elastos::Droid::R::styleable::Window));

        AutoPtr<Entry> ent = AttributeCache::GetInstance()->Get(pkg, theme, attrIds, mCurrentUserId);
        if (ent == NULL) {
            // Whoops!  App doesn't exist.  Um.  Okay.  We'll just
            // pretend like we didn't see that.
            return NOERROR;
        }
        // if (DEBUG_STARTING_WINDOW) Slog.v(TAG, "Translucent="
        //         + ent.array.getBoolean(
        //                 com.android.internal.R.styleable.Window_windowIsTranslucent, false)
        //         + " Floating="
        //         + ent.array.getBoolean(
        //                 com.android.internal.R.styleable.Window_windowIsFloating, false)
        //         + " ShowWallpaper="
        //         + ent.array.getBoolean(
        //                 com.android.internal.R.styleable.Window_windowShowWallpaper, false));
        Boolean res;
        ent->mArray->GetBoolean(Elastos::Droid::R::styleable::Window_windowIsTranslucent,
                FALSE, &res);
        if (res) {
            return NOERROR;
        }
        ent->mArray->GetBoolean(Elastos::Droid::R::styleable::Window_windowIsFloating,
                FALSE, &res);
        if (res) {
            return NOERROR;
        }
        ent->mArray->GetBoolean(Elastos::Droid::R::styleable::Window_windowShowWallpaper,
                FALSE, &res);
        if (res) {
            if (mWallpaperTarget == NULL) {
                // If this theme is requesting a wallpaper, and the wallpaper
                // is not curently visible, then this effectively serves as
                // an opaque window and our starting window transition animation
                // can still work.  We just need to make sure the starting window
                // is also showing the wallpaper.
                windowFlags |= IWindowManagerLayoutParams::FLAG_SHOW_WALLPAPER;
            }
            else {
                return NOERROR;
            }
        }
    }

    if (DEBUG_STARTING_WINDOW) {
        Slogger::V(TAG, "Creating StartingData");
    }
    mStartingIconInTransition = TRUE;
    wtoken->mStartingData = new StartingData(
            pkg, theme, compatInfo, nonLocalizedLabel, labelRes, icon, logo, windowFlags);

    AutoPtr<IMessage> msg;
    mH->ObtainMessage(H::ADD_STARTING, (IObject*)wtoken.Get(), (IMessage**)&msg);

    // Note: we really want to do sendMessageAtFrontOfQueue() because we
    // want to process the message ASAP, before any other queued
    // messages.
    Boolean result;
    return mH->SendMessageAtFrontOfQueue(msg, &result);
}

void CWindowManagerService::RemoveAppStartingWindow(
    /* [in] */ IBinder* token)
{
    synchronized (mWindowMapLock) {
        AutoPtr<AppWindowToken> wtoken;
        HashMap<AutoPtr<IBinder>, AutoPtr<WindowToken> >::Iterator it = mTokenMap.Find(token);
        if (it != mTokenMap.End()) {
            wtoken = it->mSecond->mAppWindowToken;
        }
        if (wtoken->mStartingWindow != NULL) {
            ScheduleRemoveStartingWindowLocked(wtoken);
        }
    }
}

ECode CWindowManagerService::SetAppWillBeHidden(
    /* [in] */ IBinder* token)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("SetAppWillBeHidden()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    AutoPtr<AppWindowToken> wtoken;

    synchronized (mWindowMapLock) {
        wtoken = FindAppWindowToken(token);
        if (wtoken != NULL) {
            wtoken->mWillBeHidden = true;
        }
    }
    return NOERROR;
}

void CWindowManagerService::SetAppFullscreen(
    /* [in] */ IBinder* token,
    /* [in] */ Boolean toOpaque)
{
    synchronized (mWindowMapLock) {
        AutoPtr<AppWindowToken> atoken = FindAppWindowToken(token);
        if (atoken != NULL) {
            atoken->mAppFullscreen = toOpaque;
            SetWindowOpaqueLocked(token, toOpaque);
            RequestTraversalLocked();
        }
    }
}

void CWindowManagerService::SetWindowOpaque(
    /* [in] */ IBinder* token,
    /* [in] */ Boolean isOpaque)
{
    synchronized (mWindowMapLock) {
        SetWindowOpaqueLocked(token, isOpaque);
    }
}

void CWindowManagerService::SetWindowOpaqueLocked(
    /* [in] */ IBinder* token,
    /* [in] */ Boolean isOpaque)
{
    AutoPtr<AppWindowToken> wtoken = FindAppWindowToken(token);
    if (wtoken != NULL) {
        AutoPtr<WindowState> win = wtoken->FindMainWindow();
        if (win != NULL) {
            win->mWinAnimator->SetOpaqueLocked(isOpaque);
        }
    }
}

Boolean CWindowManagerService::SetTokenVisibilityLocked(
    /* [in] */ AppWindowToken* wtoken,
    /* [in] */ IWindowManagerLayoutParams* lp,
    /* [in] */ Boolean visible,
    /* [in] */ Int32 transit,
    /* [in] */ Boolean performLayout,
    /* [in] */ Boolean isVoiceInteraction)
{
    Boolean delayed = FALSE;

    if (wtoken->mClientHidden == visible) {
        wtoken->mClientHidden = !visible;
        wtoken->SendAppVisibilityToClients();
    }

    wtoken->mWillBeHidden = FALSE;
    if (wtoken->mHidden == visible) {
        Boolean changed = FALSE;
        // if (DEBUG_APP_TRANSITIONS) Slog.v(
        //         TAG, "Changing app " + wtoken + " hidden=" + wtoken.hidden
        //         + " performLayout=" + performLayout);

        Boolean runningAppAnimation = FALSE;

        if (transit != AppTransition::TRANSIT_UNSET) {
            if (wtoken->mAppAnimator->mAnimation == AppWindowAnimator::sDummyAnimation) {
                wtoken->mAppAnimator->mAnimation = NULL;
            }
            if (ApplyAnimationLocked(wtoken, lp, transit, visible, isVoiceInteraction)) {
                delayed = runningAppAnimation = TRUE;
            }
            AutoPtr<WindowState> window = wtoken->FindMainWindow();
            //TODO (multidisplay): Magnification is supported only for the default display.
            if (window != NULL && mAccessibilityController != NULL
                    && window->GetDisplayId() == IDisplay::DEFAULT_DISPLAY) {
                mAccessibilityController->OnAppWindowTransitionLocked(window, transit);
            }
            changed = TRUE;
        }

        WindowList::Iterator it = wtoken->mAllAppWindows.Begin();
        for (; it != wtoken->mAllAppWindows.End(); ++it) {
            AutoPtr<WindowState> win = *it;
            if (win == wtoken->mStartingWindow) {
                continue;
            }

            // Slog.i(TAG, "Window " + win + ": vis=" + win.isVisible());
            // win.dump("  ");
            if (visible) {
                if (!win->IsVisibleNow()) {
                    if (!runningAppAnimation) {
                        win->mWinAnimator->ApplyAnimationLocked(
                                IWindowManagerPolicy::TRANSIT_ENTER, TRUE);
                        //TODO (multidisplay): Magnification is supported only for the default
                        if (mAccessibilityController != NULL
                                && win->GetDisplayId() == IDisplay::DEFAULT_DISPLAY) {
                            mAccessibilityController->OnWindowTransitionLocked(win,
                                    IWindowManagerPolicy::TRANSIT_ENTER);
                        }
                    }
                    changed = TRUE;
                    AutoPtr<DisplayContent> displayContent = win->GetDisplayContent();
                    if (displayContent != NULL) {
                        displayContent->mLayoutNeeded = TRUE;
                    }
                }
            }
            else if (win->IsVisibleNow()) {
                if (!runningAppAnimation) {
                    win->mWinAnimator->ApplyAnimationLocked(
                            IWindowManagerPolicy::TRANSIT_EXIT, FALSE);
                    //TODO (multidisplay): Magnification is supported only for the default
                    if (mAccessibilityController != NULL
                            && win->GetDisplayId() == IDisplay::DEFAULT_DISPLAY) {
                        mAccessibilityController->OnWindowTransitionLocked(win,
                                IWindowManagerPolicy::TRANSIT_ENTER);
                    }
                }
                changed = TRUE;
                AutoPtr<DisplayContent> displayContent = win->GetDisplayContent();
                if (displayContent != NULL) {
                    displayContent->mLayoutNeeded = TRUE;
                }
            }
        }

        wtoken->mHidden = wtoken->mHiddenRequested = !visible;
        if (!visible) {
            UnsetAppFreezingScreenLocked(wtoken, TRUE, TRUE);
        }
        else {
            // If we are being set visible, and the starting window is
            // not yet displayed, then make sure it doesn't get displayed.
            AutoPtr<WindowState> swin = wtoken->mStartingWindow;
            if (swin != NULL && !swin->IsDrawnLw()) {
                swin->mPolicyVisibility = FALSE;
                swin->mPolicyVisibilityAfterAnim = FALSE;
             }
        }

        // if (DEBUG_APP_TRANSITIONS) Slog.v(TAG, "setTokenVisibilityLocked: " + wtoken
        //         + ": hidden=" + wtoken.hidden + " hiddenRequested="
        //         + wtoken.hiddenRequested);

        if (changed) {
            mInputMonitor->SetUpdateInputWindowsNeededLw();
            if (performLayout) {
                UpdateFocusedWindowLocked(UPDATE_FOCUS_WILL_PLACE_SURFACES,
                        FALSE /*updateInputWindows*/);
                PerformLayoutAndPlaceSurfacesLocked();
            }
            mInputMonitor->UpdateInputWindowsLw(FALSE /*force*/);
        }
    }

    if (wtoken->mAppAnimator->mAnimation != NULL) {
        delayed = TRUE;
    }

    WindowList::ReverseIterator rit = wtoken->mAllAppWindows.RBegin();
    for (; rit != wtoken->mAllAppWindows.REnd(); ++rit) {
        if ((*rit)->mWinAnimator->IsWindowAnimating()) {
            delayed = TRUE;
        }
    }

    return delayed;
}

ECode CWindowManagerService::SetAppVisibility(
    /* [in] */ IBinder* token,
    /* [in] */ Boolean visible)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("SetAppVisibility()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    AutoPtr<AppWindowToken> wtoken;

    synchronized (mWindowMapLock) {
        wtoken = FindAppWindowToken(token);
        if (wtoken == NULL) {
            Slogger::W(TAG, "Attempted to set visibility of non-existing app token: %p", token);
            return NOERROR;
        }

        // if (DEBUG_APP_TRANSITIONS || DEBUG_ORIENTATION) {
        //     Slog.v(TAG, "setAppVisibility(" + token + ", visible=" + visible + "): " + mAppTransition +
        //             " hidden=" + wtoken.hidden + " hiddenRequested=" +
        //             wtoken.hiddenRequested, HIDE_STACK_CRAWLS ?
        //             null : new RuntimeException("here").fillInStackTrace());
        // }

        // If we are preparing an app transition, then delay changing
        // the visibility of this token until we execute that transition.
        if (OkToDisplay() && mAppTransition->IsTransitionSet()) {
            wtoken->mHiddenRequested = !visible;

            if (!wtoken->mStartingDisplayed) {
                // if (DEBUG_APP_TRANSITIONS) Slog.v(
                //         TAG, "Setting dummy animation on: " + wtoken);
                wtoken->mAppAnimator->SetDummyAnimation();
            }
            mOpeningApps.Remove(wtoken);
            mClosingApps.Remove(wtoken);
            wtoken->mWaitingToShow = wtoken->mWaitingToHide = FALSE;
            wtoken->mInPendingTransaction = TRUE;
            if (visible) {
                mOpeningApps.PushBack(wtoken);
                wtoken->mStartingDisplayed = FALSE;
                wtoken->mEnteringAnimation = TRUE;

                // If the token is currently hidden (should be the
                // common case), then we need to set up to wait for
                // its windows to be ready.
                if (wtoken->mHidden) {
                    wtoken->mAllDrawn = FALSE;
                    wtoken->mDeferClearAllDrawn = FALSE;
                    wtoken->mWaitingToShow = TRUE;

                    if (wtoken->mClientHidden) {
                        // In the case where we are making an app visible
                        // but holding off for a transition, we still need
                        // to tell the client to make its windows visible so
                        // they get drawn.  Otherwise, we will wait on
                        // performing the transition until all windows have
                        // been drawn, they never will be, and we are sad.
                        wtoken->mClientHidden = FALSE;
                        wtoken->SendAppVisibilityToClients();
                    }
                }
            }
            else {
                mClosingApps.PushBack(wtoken);
                wtoken->mEnteringAnimation = FALSE;

                // If the token is currently visible (should be the
                // common case), then set up to wait for it to be hidden.
                if (!wtoken->mHidden) {
                    wtoken->mWaitingToHide = TRUE;
                }
            }
            if (mAppTransition->GetAppTransition() == AppTransition::TRANSIT_TASK_OPEN_BEHIND) {
                // We're launchingBehind, add the launching activity to mOpeningApps.
                AutoPtr<WindowState> win = FindFocusedWindowLocked(GetDefaultDisplayContentLocked());
                if (win != NULL) {
                    AutoPtr<AppWindowToken> focusedToken = win->mAppToken;
                    if (focusedToken != NULL) {
                        if (DEBUG_APP_TRANSITIONS) {
                            Slogger::D(TAG, "TRANSIT_TASK_OPEN_BEHIND, %p adding  to mOpeningApps", focusedToken.Get());
                        }
                        // Force animation to be loaded.
                        focusedToken->mHidden = TRUE;
                        mOpeningApps.PushBack(focusedToken);
                    }
                }
            }
            return NOERROR;
        }

        Int64 origId = Binder::ClearCallingIdentity();
        SetTokenVisibilityLocked(wtoken, NULL, visible, AppTransition::TRANSIT_UNSET,
                TRUE, wtoken->mVoiceInteraction);
        wtoken->UpdateReportedVisibilityLocked();
        Binder::RestoreCallingIdentity(origId);
    }

    return NOERROR;
}

void CWindowManagerService::UnsetAppFreezingScreenLocked(
    /* [in] */ AppWindowToken* wtoken,
    /* [in] */ Boolean unfreezeSurfaceNow,
    /* [in] */ Boolean force)
{
    if (wtoken->mAppAnimator->mFreezingScreen) {
        // if (DEBUG_ORIENTATION) Slog.v(TAG, "Clear freezing of " + wtoken
        //         + " force=" + force);
        Boolean unfrozeWindows = FALSE;
        WindowList::Iterator it = wtoken->mAllAppWindows.Begin();
        for (; it != wtoken->mAllAppWindows.End(); ++it) {
            AutoPtr<WindowState> w = *it;
            if (w->mAppFreezing) {
                w->mAppFreezing = FALSE;
                if (w->mHasSurface && !w->mOrientationChanging) {
                    // if (DEBUG_ORIENTATION) Slog.v(TAG, "set mOrientationChanging of " + w);
                    w->mOrientationChanging = TRUE;
                    mInnerFields->mOrientationChangeComplete = FALSE;
                }
                w->mLastFreezeDuration = 0;
                unfrozeWindows = TRUE;
                AutoPtr<DisplayContent> displayContent = w->GetDisplayContent();
                if (displayContent != NULL) {
                    displayContent->mLayoutNeeded = TRUE;
                }
            }
        }
        if (force || unfrozeWindows) {
            if (DEBUG_ORIENTATION) Slogger::V(TAG, "No longer freezing: %p", wtoken);
            wtoken->mAppAnimator->mFreezingScreen = FALSE;
            wtoken->mAppAnimator->mLastFreezeDuration = (Int32)(SystemClock::GetElapsedRealtime()
                    - mDisplayFreezeTime);
            mAppsFreezingScreen--;
            mLastFinishedFreezeSource = wtoken;
        }
        if (unfreezeSurfaceNow) {
            if (unfrozeWindows) {
                PerformLayoutAndPlaceSurfacesLocked();
            }
            StopFreezingDisplayLocked();
        }
    }
}

void CWindowManagerService::StartAppFreezingScreenLocked(
    /* [in] */ AppWindowToken* wtoken)
{
    // if (DEBUG_ORIENTATION) {
    //     RuntimeException e = null;
    //     if (!HIDE_STACK_CRAWLS) {
    //         e = new RuntimeException();
    //         e.fillInStackTrace();
    //     }
    //     Slog.i(TAG, "Set freezing of " + wtoken.appToken
    //             + ": hidden=" + wtoken.hidden + " freezing="
    //             + wtoken.mAppAnimator.freezingScreen, e);
    // }
    if (!wtoken->mHiddenRequested) {
        if (!wtoken->mAppAnimator->mFreezingScreen) {
            wtoken->mAppAnimator->mFreezingScreen = TRUE;
            wtoken->mAppAnimator->mLastFreezeDuration = 0;
            mAppsFreezingScreen++;
            if (mAppsFreezingScreen == 1) {
                StartFreezingDisplayLocked(FALSE, 0, 0);

                mH->RemoveMessages(H::APP_FREEZE_TIMEOUT);
                Boolean result;
                mH->SendEmptyMessageDelayed(H::APP_FREEZE_TIMEOUT, 5000, &result);
            }
        }

        WindowList::Iterator it = wtoken->mAllAppWindows.Begin();
        for (; it != wtoken->mAllAppWindows.End(); ++it) {
            AutoPtr<WindowState> w = *it;
            w->mAppFreezing = TRUE;
        }
    }
}

ECode CWindowManagerService::StartAppFreezingScreen(
    /* [in] */ IBinder* token,
    /* [in] */ Int32 configChanges)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("StartAppFreezingScreen()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized(mWindowMapLock) {
        if (configChanges == 0 && OkToDisplay()) {
            // if (DEBUG_ORIENTATION) Slog.v(TAG, "Skipping set freeze of " + token);
            return NOERROR;
        }

        AutoPtr<AppWindowToken> wtoken = FindAppWindowToken(token);
        if (wtoken == NULL || wtoken->mAppToken == NULL) {
            Slogger::W(TAG, "Attempted to freeze screen with non-existing app token: %p"
                    , wtoken.Get());
            return NOERROR;
        }
        Int64 origId = Binder::ClearCallingIdentity();
        StartAppFreezingScreenLocked(wtoken);
        Binder::RestoreCallingIdentity(origId);
    }

    return NOERROR;
}

ECode CWindowManagerService::StopAppFreezingScreen(
    /* [in] */ IBinder* token,
    /* [in] */ Boolean force)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("StopAppFreezingScreen()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized(mWindowMapLock) {
        AutoPtr<AppWindowToken> wtoken = FindAppWindowToken(token);
        if (wtoken == NULL || wtoken->mAppToken == NULL) {
            return NOERROR;
        }
        Int64 origId = Binder::ClearCallingIdentity();
        // if (DEBUG_ORIENTATION) Slog.v(TAG, "Clear freezing of " + token
        //         + ": hidden=" + wtoken.hidden + " freezing=" + wtoken.mAppAnimator.freezingScreen);
        UnsetAppFreezingScreenLocked(wtoken, TRUE, force);
        Binder::RestoreCallingIdentity(origId);
    }

    return NOERROR;
}

void CWindowManagerService::RemoveAppFromTaskLocked(
    /* [in] */ AppWindowToken* wtoken)
{
    wtoken->RemoveAllWindows();

    AutoPtr<IInterface> value;
    mTaskIdToTask->Get(wtoken->mGroupId, (IInterface**)&value);
    AutoPtr<Task> task = (Task*)(IObject*)value.Get();
    if (task != NULL) {
        if (!task->RemoveAppToken(wtoken)) {
            Slogger::E(TAG, "removeAppFromTaskLocked: token=%p not found.", wtoken);
        }
    }
}

ECode CWindowManagerService::RemoveAppToken(
    /* [in] */ IBinder* token)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("RemoveAppToken()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    AutoPtr<AppWindowToken> wtoken;
    AutoPtr<AppWindowToken> startingToken;
    Boolean delayed = FALSE;

    Int64 origId = Binder::ClearCallingIdentity();
    synchronized(mWindowMapLock) {
        AutoPtr<WindowToken> basewtoken;
        HashMap<AutoPtr<IBinder>, AutoPtr<WindowToken> >::Iterator tokenIt
                = mTokenMap.Find(token);
        if (tokenIt != mTokenMap.End()) {
            basewtoken = tokenIt->mSecond;
            mTokenMap.Erase(tokenIt);
        }
        if (basewtoken != NULL && (wtoken = basewtoken->mAppWindowToken) != NULL) {
            if (DEBUG_APP_TRANSITIONS) Slogger::V(TAG, "Removing app token: %p", wtoken.Get());
            delayed = SetTokenVisibilityLocked(wtoken, NULL, FALSE,
                    AppTransition::TRANSIT_UNSET, TRUE, wtoken->mVoiceInteraction);
            wtoken->mInPendingTransaction = FALSE;
            mOpeningApps.Remove(wtoken);
            wtoken->mWaitingToShow = FALSE;
            List< AutoPtr<AppWindowToken> >::Iterator it = mClosingApps.Begin();
            for (; it != mClosingApps.End(); ++it) {
                if (*it == wtoken) {
                    break;
                }
            }
            if (it != mClosingApps.End()) {
                delayed = TRUE;
            }
            else if (mAppTransition->IsTransitionSet()) {
                mClosingApps.PushBack(wtoken);
                wtoken->mWaitingToHide = TRUE;
                delayed = TRUE;
            }
            if (DEBUG_APP_TRANSITIONS) {
                Slogger::V(TAG, "Removing app %p delayed=%d animation=%p animating=%d"
                        , wtoken.Get(), delayed, wtoken->mAppAnimator->mAnimation.Get(), wtoken->mAppAnimator->mAnimating);
            }
            if (DEBUG_ADD_REMOVE || DEBUG_TOKEN_MOVEMENT) {
                Slogger::V(TAG, "removeAppToken: %p delayed=%d Callers="
                        , wtoken.Get(), delayed/*, Debug.getCallers(4)*/);
            }
            AutoPtr<IInterface> value;
            mTaskIdToTask->Get(wtoken->mGroupId, (IInterface**)&value);
            AutoPtr<TaskStack> stack = (TaskStack*)(IObject*)value.Get();
            if (delayed) {
                // set the token aside because it has an active animation to be finished
                if (DEBUG_ADD_REMOVE || DEBUG_TOKEN_MOVEMENT) {
                    Slogger::V(TAG, "removeAppToken make exiting: %p", wtoken.Get());
                }
                stack->mExitingAppTokens.PushBack(wtoken);
                wtoken->mDeferRemoval = FALSE;
            }
            else {
                // Make sure there is no animation running on this token,
                // so any windows associated with it will be removed as
                // soon as their animations are complete
                wtoken->mAppAnimator->ClearAnimation();
                wtoken->mAppAnimator->mAnimating = FALSE;
                RemoveAppFromTaskLocked(wtoken);
            }

            wtoken->mRemoved = TRUE;
            if (wtoken->mStartingData != NULL) {
                startingToken = wtoken;
            }
            UnsetAppFreezingScreenLocked(wtoken, TRUE, TRUE);
            if (mFocusedApp == wtoken) {
                if (DEBUG_FOCUS_LIGHT) Slogger::V(TAG, "Removing focused app token:%p", wtoken.Get());
                mFocusedApp = NULL;
                UpdateFocusedWindowLocked(UPDATE_FOCUS_NORMAL, TRUE /*updateInputWindows*/);
                mInputMonitor->SetFocusedAppLw(NULL);
            }
        }
        else {
            Slogger::W(TAG, "Attempted to remove non-existing app token: %p", token);
        }

        if (!delayed && wtoken != NULL) {
            wtoken->UpdateReportedVisibilityLocked();
        }

        // Will only remove if startingToken non null.
        ScheduleRemoveStartingWindowLocked(startingToken);
    }
    Binder::RestoreCallingIdentity(origId);

    return NOERROR;
}

void CWindowManagerService::ScheduleRemoveStartingWindowLocked(
    /* [in] */ AppWindowToken* wtoken)
{
    Boolean hasMessages;
    if (mH->HasMessages(H::REMOVE_STARTING, (IObject*)wtoken, &hasMessages), hasMessages) {
        // Already scheduled.
        return;
    }
    if (wtoken != NULL && wtoken->mStartingWindow != NULL) {
        // if (DEBUG_STARTING_WINDOW) Slog.v(TAG, Debug.getCallers(1) +
        //         ": Schedule remove starting " + wtoken + (wtoken != null ?
        //         " startingWindow=" + wtoken.startingWindow : ""));
        AutoPtr<IMessage> msg;
        mH->ObtainMessage(H::REMOVE_STARTING, (IObject*)wtoken, (IMessage**)&msg);
        Boolean result;
        mH->SendMessageDelayed(msg, 2000, &result);
    }
}

Boolean CWindowManagerService::TmpRemoveAppWindowsLocked(
    /* [in] */ WindowToken* token)
{
    WindowList windows = token->mWindows;
    if (windows.Begin() == windows.End()) {
        return FALSE;
    }

    mWindowsChanged = TRUE;
    WindowList::Iterator it = windows.Begin();
    for (; it != windows.End(); ++it) {
        AutoPtr<WindowState> win = *it;
        if (DEBUG_WINDOW_MOVEMENT) Slogger::V(TAG, "Tmp removing app window %p", win.Get());
        win->GetWindowList()->Remove(win);
        List< AutoPtr<WindowState> >::ReverseIterator crit = win->mChildWindows.RBegin();
        for (; crit != win->mChildWindows.REnd(); ++crit) {
            AutoPtr<WindowState> cwin = *crit;
            if (DEBUG_WINDOW_MOVEMENT) Slogger::V(TAG, "Tmp removing child window %p", cwin.Get());
            cwin->GetWindowList()->Remove(cwin);
        }
    }
    return TRUE;
}

void CWindowManagerService::DumpAppTokensLocked()
{
    Int32 numStacks;
    mStackIdToStack->GetSize(&numStacks);
    for (Int32 stackNdx = 0; stackNdx < numStacks; ++stackNdx) {
        AutoPtr<IInterface> value;
        mStackIdToStack->ValueAt(stackNdx, (IInterface**)&value);
        AutoPtr<TaskStack> stack = (TaskStack*)(IObject*)value.Get();
        Slogger::V(TAG, "  Stack #%d tasks from bottom to top:", stack->mStackId);
        AutoPtr<List<AutoPtr<Task> > > tasks = stack->GetTasks();
        List<AutoPtr<Task> >::Iterator tasksIt = tasks->Begin();
        for (; tasksIt != tasks->End(); ++tasksIt) {
            AutoPtr<Task> task = *tasksIt;
            Slogger::V(TAG, "    Task #%d activities from bottom to top:", task->mTaskId);
            AppTokenList tokens = task->mAppTokens;
            AppTokenList::Iterator tokensIt = tokens.Begin();
            for (; tokensIt != tokens.End(); ++tokensIt) {
                Slogger::V(TAG, "      activity # : %p"/*, tokenNdx*/, (*tokensIt)->mToken.Get());
            }
        }
    }
}

void CWindowManagerService::DumpWindowsLocked()
{
    Int32 numDisplays;
    mDisplayContents->GetSize(&numDisplays);
    for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
        AutoPtr<IInterface> value;
        mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
        AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
        Slogger::V(TAG, " Display #%d", displayContent->GetDisplayId());
        AutoPtr<WindowList> windows = displayContent->GetWindowList();
        WindowList::ReverseIterator rit = windows->RBegin();
        for (; rit != windows->REnd(); ++rit) {
            Slogger::V(TAG, "  #: %p"/*, winNdx*/, (*rit).Get());
        }
    }
}

List<AutoPtr<WindowState> >::Iterator CWindowManagerService::FindAppWindowInsertionPointLocked(
    /* [in] */ AppWindowToken* target)
{
    AutoPtr<WindowList> windows = new List<AutoPtr<WindowState> >();
    Int32 taskId = target->mGroupId;
    AutoPtr<IInterface> value;
    mTaskIdToTask->Get(taskId, (IInterface**)&value);
    AutoPtr<Task> targetTask = (Task*)(IObject*)value.Get();
    if (targetTask == NULL) {
        Slogger::W(TAG, "findAppWindowInsertionPointLocked: no Task for %p taskId=%d"
                , target, taskId);
        return windows->Begin();
    }
    AutoPtr<DisplayContent> displayContent = targetTask->GetDisplayContent();
    if (displayContent == NULL) {
        Slogger::W(TAG, "findAppWindowInsertionPointLocked: no DisplayContent for %p", target);
        return windows->Begin();
    }
    windows = displayContent->GetWindowList();

    Boolean found = FALSE;
    AutoPtr<List<AutoPtr<Task> > > tasks = displayContent->GetTasks();
    List<AutoPtr<Task> >::ReverseIterator taskRit = tasks->RBegin();
    for (; taskRit != tasks->REnd(); ++taskRit) {
        AutoPtr<Task> task = *taskRit;
        if (!found && task->mTaskId != taskId) {
            continue;
        }
        AppTokenList tokens = task->mAppTokens;
        AppTokenList::ReverseIterator tokenRit = tokens.RBegin();
        for (; tokenRit != tokens.REnd(); ++tokenRit) {
            AutoPtr<AppWindowToken> wtoken = *tokenRit;
            if (!found && wtoken.Get() == target) {
                found = TRUE;
            }
            if (found) {
                // Find the first app token below the new position that has
                // a window displayed.
                if (DEBUG_REORDER) Slogger::V(TAG, "Looking for lower windows in %p", wtoken->mToken.Get());
                if (wtoken->mSendingToBottom) {
                    if (DEBUG_REORDER) Slogger::V(TAG, "Skipping token -- currently sending to bottom");
                    continue;
                }
                WindowList::ReverseIterator winRit = wtoken->mWindows.RBegin();
                for (; winRit != wtoken->mWindows.REnd(); ++winRit) {
                    AutoPtr<WindowState> win = *winRit;
                    WindowList::ReverseIterator childWinRit = win->mChildWindows.RBegin();
                    for (; childWinRit != win->mChildWindows.REnd(); ++childWinRit) {
                        AutoPtr<WindowState> cwin = *childWinRit;
                        if (cwin->mSubLayer >= 0) {
                            WindowList::ReverseIterator posRit = windows->RBegin();
                            for (; posRit != windows->REnd(); ++posRit) {
                                if (*posRit == cwin) {
                                    // if (DEBUG_REORDER) slog.v(TAG, "Found child win @" + (pos + 1));
                                    return posRit.GetBase();
                                }
                            }
                        }
                    }
                    WindowList::ReverseIterator posRit = windows->RBegin();
                    for (; posRit != windows->REnd(); ++posRit) {
                        if (*posRit == win) {
                            // if (DEBUG_REORDER) Slog.v(TAG, "Found win @" + (pos + 1));
                            return posRit.GetBase();
                        }
                    }
                }
            }
        }
    }
    // Never put an app window underneath wallpaper.
    WindowList::ReverseIterator posRit = windows->RBegin();
    for (; posRit != windows->REnd(); ++posRit) {
        if ((*posRit)->mIsWallpaper) {
            // if (DEBUG_REORDER) Slog.v(TAG, "Found wallpaper @" + pos);
            return posRit.GetBase();
        }
    }
    return windows->Begin();
}

List<AutoPtr<WindowState> >::Iterator CWindowManagerService::ReAddWindowLocked(
    /* [in] */ List< AutoPtr<WindowState> >::Iterator indexIt,
    /* [in] */ WindowState* win)
{
    AutoPtr<WindowList> windows = win->GetWindowList();
    Boolean added = FALSE;
    WindowList::Iterator cwIt = win->mChildWindows.Begin();
    for (; cwIt != win->mChildWindows.End(); cwIt++) {
        AutoPtr<WindowState> cwin = *cwIt;
        if (!added && cwin->mSubLayer >= 0) {
            // if (DEBUG_WINDOW_MOVEMENT) Slog.v(TAG, "Re-adding child window at "
            //         + index + ": " + cwin);
            win->mRebuilding = FALSE;
            windows->Insert(indexIt, win);
            added = TRUE;
        }
        // if (DEBUG_WINDOW_MOVEMENT) Slog.v(TAG, "Re-adding window at "
        //         + index + ": " + cwin);
        cwin->mRebuilding = FALSE;
        windows->Insert(indexIt, cwin);
    }
    if (!added) {
        // if (DEBUG_WINDOW_MOVEMENT) Slog.v(TAG, "Re-adding window at "
        //         + index + ": " + win);
        win->mRebuilding = FALSE;
        windows->Insert(indexIt, win);
    }
    mWindowsChanged = TRUE;
    return indexIt;
}

List<AutoPtr<WindowState> >::Iterator CWindowManagerService::ReAddAppWindowsLocked(
    /* [in] */ DisplayContent* displayContent,
    /* [in] */ List< AutoPtr<WindowState> >::Iterator indexIt,
    /* [in] */ WindowToken* token)
{
    WindowList::Iterator it = token->mWindows.Begin();
    for (; it != token->mWindows.End(); ++it) {
        AutoPtr<WindowState> win = *it;
        AutoPtr<DisplayContent> winDisplayContent = win->GetDisplayContent();
        if (win->mDisplayContent.Get() == displayContent || winDisplayContent == NULL) {
            win->mDisplayContent = displayContent;
            indexIt = ReAddWindowLocked(indexIt, *it);
        }
    }
    return indexIt;
}

void CWindowManagerService::TmpRemoveTaskWindowsLocked(
    /* [in] */ Task* task)
{
    AppTokenList tokens = task->mAppTokens;
    AppTokenList::ReverseIterator rit = tokens.RBegin();
    for (; rit != tokens.REnd(); ++rit) {
        TmpRemoveAppWindowsLocked(*rit);
    }
}

void CWindowManagerService::MoveStackWindowsLocked(
    /* [in] */ DisplayContent* displayContent)
{
    // First remove all of the windows from the list.
    AutoPtr<List<AutoPtr<Task> > > tasks = displayContent->GetTasks();
    List<AutoPtr<Task> >::Iterator taskIt = tasks->Begin();
    for (; taskIt != tasks->End(); ++taskIt) {
        TmpRemoveTaskWindowsLocked(*taskIt);
    }

    // And now add them back at the correct place.
    // Where to start adding?
    taskIt = tasks->Begin();
    for (; taskIt != tasks->End(); ++taskIt) {
        AppTokenList tokens = (*taskIt)->mAppTokens;
        if (tokens.Begin() == tokens.End()) {
            continue;
        }
        WindowList::Iterator posIt = FindAppWindowInsertionPointLocked(*(tokens.Begin()));
        AppTokenList::Iterator tokenIt = tokens.Begin();
        for (; tokenIt != tokens.End(); ++tokenIt) {
            AutoPtr<AppWindowToken> wtoken = *tokenIt;
            if (wtoken != NULL) {
                WindowList::Iterator newPosIt = ReAddAppWindowsLocked(displayContent, posIt, wtoken);
                if ((*newPosIt) != (*posIt)) {
                    displayContent->mLayoutNeeded = TRUE;
                }
                posIt = newPosIt;
            }
        }
    }

    if (!UpdateFocusedWindowLocked(UPDATE_FOCUS_WILL_PLACE_SURFACES,
            FALSE /*updateInputWindows*/)) {
        AssignLayersLocked(displayContent->GetWindowList());
    }

    mInputMonitor->SetUpdateInputWindowsNeededLw();
    PerformLayoutAndPlaceSurfacesLocked();
    mInputMonitor->UpdateInputWindowsLw(FALSE /*force*/);

    //dump();
}

void CWindowManagerService::MoveTaskToTop(
    /* [in] */ Int32 taskId)
{
    Int64 origId = Binder::ClearCallingIdentity();
    // try {
    synchronized (mWindowMapLock) {
        AutoPtr<IInterface> value;
        mTaskIdToTask->Get(taskId, (IInterface**)&value);
        AutoPtr<Task> task = (Task*)(IObject*)value.Get();
        if (task == NULL) {
            // Normal behavior, addAppToken will be called next and task will be created.
            return;
        }
        AutoPtr<TaskStack> stack = task->mStack;
        AutoPtr<DisplayContent> displayContent = task->GetDisplayContent();
        displayContent->MoveStack(stack, TRUE);
        if (displayContent->mIsDefaultDisplay) {
            AutoPtr<TaskStack> homeStack = displayContent->GetHomeStack();
            if (homeStack != stack) {
                // When a non-home stack moves to the top, the home stack moves to the
                // bottom.
                displayContent->MoveStack(homeStack, FALSE);
            }
        }
        stack->MoveTaskToTop(task);
    }
    // } finally {
    //     Binder.restoreCallingIdentity(origId);
    // }
    Binder::RestoreCallingIdentity(origId);
}

void CWindowManagerService::MoveTaskToBottom(
    /* [in] */ Int32 taskId)
{
    Int64 origId = Binder::ClearCallingIdentity();
    // try {
    synchronized (mWindowMapLock) {
        AutoPtr<IInterface> value;
        mTaskIdToTask->Get(taskId, (IInterface**)&value);
        AutoPtr<Task> task = (Task*)(IObject*)value.Get();
        if (task == NULL) {
            Slogger::E(TAG, "moveTaskToBottom: taskId=%d not found in mTaskIdToTask", taskId);
            return;
        }
        AutoPtr<TaskStack> stack = task->mStack;
        stack->MoveTaskToBottom(task);
        MoveStackWindowsLocked(stack->GetDisplayContent());
    }
    // } finally {
    //     Binder.restoreCallingIdentity(origId);
    // }
    Binder::RestoreCallingIdentity(origId);
}

void CWindowManagerService::AttachStack(
    /* [in] */ Int32 stackId,
    /* [in] */ Int32 displayId)
{
    Int64 origId = Binder::ClearCallingIdentity();
    // try {
    synchronized (mWindowMapLock) {
        AutoPtr<IInterface> value;
        mDisplayContents->Get(displayId, (IInterface**)&value);
        AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
        if (displayContent != NULL) {
            AutoPtr<IInterface> value;
            mStackIdToStack->Get(stackId, (IInterface**)&value);
            AutoPtr<TaskStack> stack = (TaskStack*)(IObject*)value.Get();
            if (stack == NULL) {
                if (DEBUG_STACK) Slogger::D(TAG, "attachStack: stackId=%d", stackId);
                stack = new TaskStack(this, stackId);
                mStackIdToStack->Put(stackId, (IObject*)stack);
            }
            stack->AttachDisplayContent(displayContent);
            displayContent->AttachStack(stack);
            MoveStackWindowsLocked(displayContent);
            AutoPtr<WindowList> windows = displayContent->GetWindowList();
            WindowList::ReverseIterator rit = windows->RBegin();
            for (; rit != windows->REnd(); ++rit) {
                (*rit)->ReportResized();
            }
        }
    }
    // } finally {
    //     Binder.restoreCallingIdentity(origId);
    // }
    Binder::RestoreCallingIdentity(origId);
}

void CWindowManagerService::DetachStackLocked(
    /* [in] */ DisplayContent* displayContent,
    /* [in] */ TaskStack* stack)
{
    displayContent->DetachStack(stack);
    stack->DetachDisplay();
}

void CWindowManagerService::DetachStack(
    /* [in] */ Int32 stackId)
{
    synchronized (mWindowMapLock) {
        AutoPtr<IInterface> value;
        mStackIdToStack->Get(stackId, (IInterface**)&value);
        AutoPtr<TaskStack> stack = (TaskStack*)(IObject*)value.Get();
        if (stack != NULL) {
            AutoPtr<DisplayContent> displayContent = stack->GetDisplayContent();
            if (displayContent != NULL) {
                if (stack->IsAnimating()) {
                    stack->mDeferDetach = TRUE;
                    return;
                }
                DetachStackLocked(displayContent, stack);
            }
        }
    }
}

void CWindowManagerService::RemoveStack(
    /* [in] */ Int32 stackId)
{
    mStackIdToStack->Remove(stackId);
}

void CWindowManagerService::RemoveTaskLocked(
    /* [in] */ Task* task)
{
    Int32 taskId = task->mTaskId;
    AutoPtr<TaskStack> stack = task->mStack;
    if (stack->IsAnimating()) {
        if (DEBUG_STACK) Slogger::I(TAG, "removeTask: deferring removing taskId=%d", taskId);
        task->mDeferRemoval = TRUE;
        return;
    }
    if (DEBUG_STACK) Slogger::I(TAG, "removeTask: removing taskId=%d", taskId);
    // EventLog::WriteEvent(EventLogTags.WM_TASK_REMOVED, taskId, "removeTask");
    task->mDeferRemoval = FALSE;
    task->mStack->RemoveTask(task);
    mTaskIdToTask->Delete(task->mTaskId);
}

void CWindowManagerService::RemoveTask(
    /* [in] */ Int32 taskId)
{
    synchronized (mWindowMapLock) {
        AutoPtr<IInterface> value;
        mTaskIdToTask->Get(taskId, (IInterface**)&value);
        AutoPtr<Task> task = (Task*)(IObject*)value.Get();
        if (task == NULL) {
            if (DEBUG_STACK) Slogger::I(TAG, "removeTask: could not find taskId=%d", taskId);
            return;
        }
        RemoveTaskLocked(task);
    }
}

void CWindowManagerService::AddTask(
    /* [in] */ Int32 taskId,
    /* [in] */ Int32 stackId,
    /* [in] */ Boolean toTop)
{
    synchronized (mWindowMapLock) {
        if (DEBUG_STACK) Slogger::I(TAG, "addTask: adding taskId=%d to %d", taskId, (toTop ? "top" : "bottom"));
        AutoPtr<IInterface> value;
        mTaskIdToTask->Get(taskId, (IInterface**)&value);
        AutoPtr<Task> task = (Task*)(IObject*)value.Get();
        if (task == NULL) {
            return;
        }
        AutoPtr<IInterface> stackValue;
        mStackIdToStack->Get(stackId, (IInterface**)&stackValue);
        AutoPtr<TaskStack> stack = (TaskStack*)(IObject*)stackValue.Get();
        stack->AddTask(task, toTop);
        AutoPtr<DisplayContent> displayContent = stack->GetDisplayContent();
        displayContent->mLayoutNeeded = TRUE;
        PerformLayoutAndPlaceSurfacesLocked();
    }
}

ECode CWindowManagerService::ResizeStack(
    /* [in] */ Int32 stackId,
    /* [in] */ IRect* bounds)
{
    synchronized (mWindowMapLock) {
        AutoPtr<IInterface> stackValue;
        mStackIdToStack->Get(stackId, (IInterface**)&stackValue);
        AutoPtr<TaskStack> stack = (TaskStack*)(IObject*)stackValue.Get();
        if (stack == NULL) {
            Slogger::E(TAG, "resizeStack: stackId %d not found.", stackId);
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }
        if (stack->SetBounds(bounds)) {
            stack->ResizeWindows();
            stack->GetDisplayContent()->mLayoutNeeded = TRUE;
            PerformLayoutAndPlaceSurfacesLocked();
        }
    }
    return NOERROR;
}

void CWindowManagerService::GetStackBounds(
    /* [in] */ Int32 stackId,
    /* [in] */ IRect* bounds)
{
    AutoPtr<IInterface> stackValue;
    mStackIdToStack->Get(stackId, (IInterface**)&stackValue);
    AutoPtr<TaskStack> stack = (TaskStack*)(IObject*)stackValue.Get();
    if (stack != NULL) {
        stack->GetBounds(bounds);
        return;
    }
    bounds->SetEmpty();
}

// -------------------------------------------------------------
// Misc IWindowSession methods
// -------------------------------------------------------------

ECode CWindowManagerService::StartFreezingScreen(
    /* [in] */ Int32 exitAnim,
    /* [in] */ Int32 enterAnim)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::FREEZE_SCREEN,
            String("startFreezingScreen()"))) {
        Slogger::E(TAG, "Requires FREEZE_SCREEN permission");
        return E_SECURITY_EXCEPTION;
        // throw new SecurityException("Requires FREEZE_SCREEN permission");
    }

    synchronized (mWindowMapLock) {
        if (!mClientFreezingScreen) {
            mClientFreezingScreen = TRUE;
            Int64 origId = Binder::ClearCallingIdentity();
            // try {
            mH->RemoveMessages(H::CLIENT_FREEZE_TIMEOUT);
            Boolean result;
            mH->SendEmptyMessageDelayed(H::CLIENT_FREEZE_TIMEOUT, 5000, &result);
            // } finally {
            //     Binder::RestoreCallingIdentity(origId);
            // }
            Binder::RestoreCallingIdentity(origId);
        }
    }
    return NOERROR;
}

ECode CWindowManagerService::StopFreezingScreen()
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::FREEZE_SCREEN,
            String("stopFreezingScreen()"))) {
        Slogger::E(TAG, "Requires FREEZE_SCREEN permission");
        return E_SECURITY_EXCEPTION;
        // throw new SecurityException("Requires FREEZE_SCREEN permission");
    }

    synchronized (mWindowMapLock) {
        if (mClientFreezingScreen) {
            mClientFreezingScreen = FALSE;
            AutoPtr<ICharSequence> cs;
            CString::New(String("client"), (ICharSequence**)&cs);
            mLastFinishedFreezeSource = IObject::Probe(cs);
            Int64 origId = Binder::ClearCallingIdentity();
            // try {
            StopFreezingDisplayLocked();
            // } finally {
            //     Binder::RestoreCallingIdentity(origId);
            // }
            Binder::RestoreCallingIdentity(origId);
        }
    }
    return NOERROR;
}

ECode CWindowManagerService::DisableKeyguard(
    /* [in] */ IBinder* token,
    /* [in] */ const String& tag)
{
    Int32 perm;
    FAIL_RETURN(mContext->CheckCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::DISABLE_KEYGUARD,
            &perm));
    if (perm != IPackageManager::PERMISSION_GRANTED) {
        //throw new SecurityException("Requires DISABLE_KEYGUARD permission");
        Slogger::E(TAG, "Requires DISABLE_KEYGUARD permission");
        return E_SECURITY_EXCEPTION;
    }

    if (token == NULL) {
        Slogger::E(TAG, "token == null");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    AutoPtr<ICharSequence> seq;
    CString::New(tag, (ICharSequence**)&seq);
    AutoPtr<ISomeArgsHelper> helper;
    CSomeArgsHelper::AcquireSingleton((ISomeArgsHelper**)&helper);
    AutoPtr<ISomeArgs> args;
    helper->Obtain((ISomeArgs**)&args);
    args->SetObjectArg(1, token);
    args->SetObjectArg(2, seq);

    AutoPtr<IMessage> msg;
    mKeyguardDisableHandler->ObtainMessage(KeyguardDisableHandler::KEYGUARD_DISABLE,
        args, (IMessage**)&msg);
    Boolean result;
    return mKeyguardDisableHandler->SendMessage(msg, &result);
}

ECode CWindowManagerService::ReenableKeyguard(
    /* [in] */ IBinder* token)
{
    Int32 perm;
    FAIL_RETURN(mContext->CheckCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::DISABLE_KEYGUARD,
            &perm));
    if (perm != IPackageManager::PERMISSION_GRANTED) {
        //throw new SecurityException("Requires DISABLE_KEYGUARD permission");
        Slogger::E(TAG, "Requires DISABLE_KEYGUARD permission");
        return E_SECURITY_EXCEPTION;
    }

    if (token == NULL) {
        Slogger::E(TAG, "token == null");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    AutoPtr<IMessage> msg;
    mKeyguardDisableHandler->ObtainMessage(KeyguardDisableHandler::KEYGUARD_REENABLE,
        token, (IMessage**)&msg);
    Boolean result;
    return mKeyguardDisableHandler->SendMessage(msg, &result);
}

ECode CWindowManagerService::ExitKeyguardSecurely(
    /* [in] */ IOnKeyguardExitResult* keyguardCallback)
{
    Int32 perm;
    FAIL_RETURN(mContext->CheckCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::DISABLE_KEYGUARD, &perm));
    if (perm != IPackageManager::PERMISSION_GRANTED) {
        //throw new SecurityException("Requires DISABLE_KEYGUARD permission");
        return E_SECURITY_EXCEPTION;
    }
    if (keyguardCallback == NULL) {
        Slogger::E(TAG, "callback == null");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    AutoPtr<IOnKeyguardExitResult> result = new SecurelyOnKeyguardExitResult(keyguardCallback);
    mPolicy->ExitKeyguardSecurely(result);
    return NOERROR;
}

ECode CWindowManagerService::InKeyguardRestrictedInputMode(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    return mPolicy->InKeyguardRestrictedKeyInputMode(result);
}

ECode CWindowManagerService::IsKeyguardLocked(
    /* [out] */ Boolean* isSecure)
{
    VALIDATE_NOT_NULL(isSecure)
    return mPolicy->IsKeyguardLocked(isSecure);
}

ECode CWindowManagerService::IsKeyguardSecure(
    /* [out] */ Boolean* isSecure)
{
    VALIDATE_NOT_NULL(isSecure)
    return mPolicy->IsKeyguardSecure(isSecure);
}

ECode CWindowManagerService::DismissKeyguard()
{
    Int32 perm;
    FAIL_RETURN(mContext->CheckCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::DISABLE_KEYGUARD, &perm));
    if (perm != IPackageManager::PERMISSION_GRANTED) {
        //throw new SecurityException("Requires DISABLE_KEYGUARD permission");
        Slogger::E(TAG, "Requires DISABLE_KEYGUARD permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized (mWindowMapLock) {
        mPolicy->DismissKeyguardLw();
    }
    return NOERROR;
}

ECode CWindowManagerService::KeyguardGoingAway(
    /* [in] */ Boolean disableWindowAnimations,
    /* [in] */ Boolean keyguardGoingToNotificationShade)
{
    Int32 perm;
    FAIL_RETURN(mContext->CheckCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::DISABLE_KEYGUARD, &perm));
    if (perm != IPackageManager::PERMISSION_GRANTED) {
        //throw new SecurityException("Requires DISABLE_KEYGUARD permission");
        Slogger::E(TAG, "Requires DISABLE_KEYGUARD permission");
        return E_SECURITY_EXCEPTION;
    }
    synchronized (mWindowMapLock) {
        mAnimator->mKeyguardGoingAway = TRUE;
        mAnimator->mKeyguardGoingAwayToNotificationShade = keyguardGoingToNotificationShade;
        mAnimator->mKeyguardGoingAwayDisableWindowAnimations = disableWindowAnimations;
        RequestTraversalLocked();
    }
    return NOERROR;
}

void CWindowManagerService::KeyguardWaitingForActivityDrawn()
{
    synchronized (mWindowMapLock) {
        mKeyguardWaitingForActivityDrawn = TRUE;
    }
}

void CWindowManagerService::NotifyActivityDrawnForKeyguard()
{
    synchronized (mWindowMapLock) {
        if (mKeyguardWaitingForActivityDrawn) {
            mPolicy->NotifyActivityDrawnForKeyguardLw();
            mKeyguardWaitingForActivityDrawn = FALSE;
        }
    }
}

void CWindowManagerService::ShowGlobalActions()
{
    mPolicy->ShowGlobalActions();
}

ECode CWindowManagerService::CloseSystemDialogs(
    /* [in] */ const String& reason)
{
    synchronized (mWindowMapLock) {
        Int32 numDisplays;
        mDisplayContents->GetSize(&numDisplays);
        for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
            AutoPtr<IInterface> value;
            mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
            AutoPtr<DisplayContent> dc = (DisplayContent*)(IObject*)value.Get();
            AutoPtr<WindowList> windows = dc->GetWindowList();
            WindowList::Iterator it = windows->Begin();
            for (; it != windows->End(); ++it) {
                AutoPtr<WindowState> w = *it;
                if (w->mHasSurface) {
                    // try {
                    w->mClient->CloseSystemDialogs(reason);
                    // } catch (RemoteException e) {
                    // }
                }
            }
        }
    }

    return NOERROR;
}

Float CWindowManagerService::FixScale(
    /* [in] */ Float scale)
{
    if (scale < 0) scale = 0;
    else if (scale > 20) scale = 20;
    return Elastos::Core::Math::Abs(scale);
}

ECode CWindowManagerService::SetAnimationScale(
    /* [in] */ Int32 which,
    /* [in] */ Float scale)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::SET_ANIMATION_SCALE,
            String("SetAnimationScale()"))) {
        //throw new SecurityException("Requires SET_ANIMATION_SCALE permission");
        Slogger::E(TAG, "Requires SET_ANIMATION_SCALE permission");
        return E_SECURITY_EXCEPTION;
    }

    scale = FixScale(scale);
    switch (which) {
        case 0: mWindowAnimationScaleSetting = scale; break;
        case 1: mTransitionAnimationScaleSetting = scale; break;
        case 2: mAnimatorDurationScaleSetting = scale; break;
    }

    // Persist setting
    Boolean result;
    return mH->SendEmptyMessage(H::PERSIST_ANIMATION_SCALE, &result);
}

ECode CWindowManagerService::SetAnimationScales(
    /* [in] */ ArrayOf<Float>* scales)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::SET_ANIMATION_SCALE,
            String("SetAnimationScale()"))) {
        //throw new SecurityException("Requires SET_ANIMATION_SCALE permission");
        Slogger::E(TAG, "Requires SET_ANIMATION_SCALE permission");
        return E_SECURITY_EXCEPTION;
    }

    if (scales != NULL) {
        Int32 len = scales->GetLength();
        if (len >= 1) {
            mWindowAnimationScaleSetting = FixScale((*scales)[0]);
        }
        if (len >= 2) {
            mTransitionAnimationScaleSetting = FixScale((*scales)[1]);
        }
        if (len >= 3) {
            mAnimatorDurationScaleSetting = FixScale((*scales)[2]);
            DispatchNewAnimatorScaleLocked(NULL);
        }
    }

    // Persist setting
    Boolean result;
    return mH->SendEmptyMessage(H::PERSIST_ANIMATION_SCALE, &result);
}

void CWindowManagerService::SetAnimatorDurationScale(
    /* [in] */ Float scale)
{
    mAnimatorDurationScaleSetting = scale;
    AutoPtr<IValueAnimatorHelper> helper;
    CValueAnimatorHelper::AcquireSingleton((IValueAnimatorHelper**)&helper);
    helper->SetDurationScale(scale);
}

Float CWindowManagerService::GetWindowAnimationScaleLocked()
{
    return mAnimationsDisabled ? 0 : mWindowAnimationScaleSetting;
}

Float CWindowManagerService::GetTransitionAnimationScaleLocked()
{
    return mAnimationsDisabled ? 0 : mTransitionAnimationScaleSetting;
}

ECode CWindowManagerService::GetAnimationScale(
    /* [in] */ Int32 which,
    /* [out] */ Float* scale)
{
    VALIDATE_NOT_NULL(scale)
    *scale = 0;

    switch (which) {
        case 0: *scale = mWindowAnimationScaleSetting;
        case 1: *scale = mTransitionAnimationScaleSetting;
        case 2: *scale = mAnimatorDurationScaleSetting;
    }
    return NOERROR;
}

ECode CWindowManagerService::GetAnimationScales(
    /* [out, callee] */ ArrayOf<Float>** scales)
{
    VALIDATE_NOT_NULL(scales)
    AutoPtr< ArrayOf<Float> > temp = ArrayOf<Float>::Alloc(3);
    (*temp)[0] = mWindowAnimationScaleSetting;
    (*temp)[1] = mTransitionAnimationScaleSetting;
    (*temp)[2] = mAnimatorDurationScaleSetting;
    *scales = temp;
    REFCOUNT_ADD(*scales);
    return NOERROR;
}

ECode CWindowManagerService::GetCurrentAnimatorScale(
    /* [out] */ Float* scale)
{
    VALIDATE_NOT_NULL(scale)
    synchronized (mWindowMapLock) {
        *scale = mAnimationsDisabled ? 0 : mAnimatorDurationScaleSetting;
    }
    return NOERROR;
}

void CWindowManagerService::DispatchNewAnimatorScaleLocked(
    /* [in] */ CSession* session)
{
    AutoPtr<IMessage> msg;
    mH->ObtainMessage(H::NEW_ANIMATOR_SCALE, (IWindowSession*)session, (IMessage**)&msg);
    msg->SendToTarget();
}

ECode CWindowManagerService::RegisterPointerEventListener(
    /* [in] */ IPointerEventListener* listener)
{
    mPointerEventDispatcher->RegisterInputEventListener(listener);
    return NOERROR;
}

ECode CWindowManagerService::UnregisterPointerEventListener(
    /* [in] */ IPointerEventListener* listener)
{
    mPointerEventDispatcher->UnregisterInputEventListener(listener);
    return NOERROR;
}

ECode CWindowManagerService::GetLidState(
    /* [out] */ Int32* lidState)
{
    VALIDATE_NOT_NULL(lidState)

    Int32 sw =mInputManager->GetSwitchState(-1, IInputDevice::SOURCE_ANY,
            CInputManagerService::SW_LID);
    if (sw > 0) {
        // Switch state: AKEY_STATE_DOWN or AKEY_STATE_VIRTUAL.
        *lidState = LID_CLOSED;
    }
    else if (sw == 0) {
        // Switch state: AKEY_STATE_UP.
        *lidState = LID_OPEN;
    }
    else {
        // Switch state: AKEY_STATE_UNKNOWN.
        *lidState = LID_ABSENT;
    }
    return NOERROR;
}

ECode CWindowManagerService::GetCameraLensCoverState(
    /* [out] */ Int32* state)
{
    VALIDATE_NOT_NULL(state)

    Int32 sw = mInputManager->GetSwitchState(-1, IInputDevice::SOURCE_ANY,
            CInputManagerService::SW_CAMERA_LENS_COVER);
    if (sw > 0) {
        // Switch state: AKEY_STATE_DOWN or AKEY_STATE_VIRTUAL.
        *state = CAMERA_LENS_COVERED;
    }
    else if (sw == 0) {
        // Switch state: AKEY_STATE_UP.
        *state = CAMERA_LENS_UNCOVERED;
    }
    else {
        // Switch state: AKEY_STATE_UNKNOWN.
        *state = CAMERA_LENS_COVER_ABSENT;
    }
    return NOERROR;
}

ECode CWindowManagerService::SwitchKeyboardLayout(
    /* [in] */ Int32 deviceId,
    /* [in] */ Int32 direction)
{
    mInputManager->SwitchKeyboardLayout(deviceId, direction);
    return NOERROR;
}

ECode CWindowManagerService::Shutdown(
    /* [in] */ Boolean confirm)
{
    ShutdownThread::Shutdown(mContext, confirm);
    return NOERROR;
}

ECode CWindowManagerService::RebootSafeMode(
    /* [in] */ Boolean confirm)
{
    ShutdownThread::RebootSafeMode(mContext, confirm);
    return NOERROR;
}

void CWindowManagerService::SetCurrentProfileIds(
    /* [in] */ ArrayOf<Int32>* currentProfileIds)
{
    synchronized (mWindowMapLock) {
        mCurrentProfileIds = currentProfileIds;
    }
}

void CWindowManagerService::SetCurrentUser(
    /* [in] */ Int32 newUserId,
    /* [in] */ ArrayOf<Int32>* currentProfileIds)
{
    synchronized (mWindowMapLock) {
        mCurrentUserId = newUserId;
        mCurrentProfileIds = currentProfileIds;
        mAppTransition->SetCurrentUser(newUserId);
        mPolicy->SetCurrentUserLw(newUserId);

        // Hide windows that should not be seen by the new user.
        Int32 numDisplays;
        mDisplayContents->GetSize(&numDisplays);
        for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
            AutoPtr<IInterface> value;
            mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
            AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
            displayContent->SwitchUserStacks(newUserId);
            RebuildAppWindowListLocked(displayContent);
        }
        PerformLayoutAndPlaceSurfacesLocked();
    }
}

Boolean CWindowManagerService::IsCurrentProfileLocked(
    /* [in] */ Int32 userId)
{
    if (userId == mCurrentUserId) return TRUE;
    for (Int32 i = 0; i < mCurrentProfileIds->GetLength(); i++) {
        if ((*mCurrentProfileIds)[i] == userId) return TRUE;
    }
    return FALSE;
}

void CWindowManagerService::EnableScreenAfterBoot()
{
    synchronized (mWindowMapLock) {
        // if (DEBUG_BOOT) {
        //     RuntimeException here = new RuntimeException("here");
        //     here.fillInStackTrace();
        //     Slog.i(TAG, "enableScreenAfterBoot: mDisplayEnabled=" + mDisplayEnabled
        //             + " mForceDisplayEnabled=" + mForceDisplayEnabled
        //             + " mShowingBootMessages=" + mShowingBootMessages
        //             + " mSystemBooted=" + mSystemBooted, here);
        // }
        if (mSystemBooted) {
            return;
        }
        mSystemBooted = TRUE;
        HideBootMessagesLocked();
        // If the screen still doesn't come up after 30 seconds, give
        // up and turn it on.
        Boolean result;
        mH->SendEmptyMessageDelayed(H::BOOT_TIMEOUT, 30*1000, &result);
    }

    mPolicy->SystemBooted();

    PerformEnableScreen();
}

ECode CWindowManagerService::EnableScreenIfNeeded()
{
    synchronized (mWindowMapLock) {
        EnableScreenIfNeededLocked();
    }
    return NOERROR;
}

void CWindowManagerService::EnableScreenIfNeededLocked()
{
    // if (DEBUG_BOOT) {
    //     RuntimeException here = new RuntimeException("here");
    //     here.fillInStackTrace();
    //     Slog.i(TAG, "enableScreenIfNeededLocked: mDisplayEnabled=" + mDisplayEnabled
    //             + " mForceDisplayEnabled=" + mForceDisplayEnabled
    //             + " mShowingBootMessages=" + mShowingBootMessages
    //             + " mSystemBooted=" + mSystemBooted, here);
    // }
    if (mDisplayEnabled) {
        return;
    }
    if (!mSystemBooted && !mShowingBootMessages) {
        return;
    }

    Boolean result;
    mH->SendEmptyMessage(H::ENABLE_SCREEN, &result);
}

void CWindowManagerService::PerformBootTimeout()
{
    synchronized (mWindowMapLock) {
        if (mDisplayEnabled) {
            return;
        }
        Slogger::W(TAG, "***** BOOT TIMEOUT: forcing display enabled");
        mForceDisplayEnabled = TRUE;
    }
    PerformEnableScreen();
}

Boolean CWindowManagerService::CheckWaitingForWindowsLocked()
{
    Boolean haveBootMsg = FALSE;
    Boolean haveApp = FALSE;
    // if the wallpaper service is disabled on the device, we're never going to have
    // wallpaper, don't bother waiting for it
    Boolean haveWallpaper = FALSE;
    AutoPtr<IResources> res;
    mContext->GetResources((IResources**)&res);
    Boolean boolValue;
    res->GetBoolean(Elastos::Droid::R::bool_::config_enableWallpaperService, &boolValue);
    Boolean wallpaperEnabled = boolValue && !mOnlyCore;
    Boolean haveKeyguard = TRUE;
    // TODO(multidisplay): Expand to all displays?
    AutoPtr<WindowList> windows = GetDefaultWindowListLocked();
    WindowList::Iterator it = windows->Begin();
    for (; it != windows->End(); ++it) {
        AutoPtr<WindowState> w = *it;
        Boolean isVisible;
        if ((w->IsVisibleLw(&isVisible), isVisible) && !w->mObscured && !w->IsDrawnLw()) {
            return TRUE;
        }
        if (w->IsDrawnLw()) {
            Int32 type;
            w->mAttrs->GetType(&type);
            if (type == IWindowManagerLayoutParams::TYPE_BOOT_PROGRESS) {
                haveBootMsg = TRUE;
            }
            else if (type == IWindowManagerLayoutParams::TYPE_APPLICATION) {
                haveApp = TRUE;
            }
            else if (type == IWindowManagerLayoutParams::TYPE_WALLPAPER) {
                haveWallpaper = TRUE;
            }
            else if (type == IWindowManagerLayoutParams::TYPE_STATUS_BAR) {
                mPolicy->IsKeyguardDrawnLw(&haveKeyguard);
            }
        }
    }

    if (DEBUG_SCREEN_ON || DEBUG_BOOT) {
        Slogger::I(TAG, "******** booted=%d msg=%d haveBoot=%d haveApp=%d haveWall=%d wallEnabled=%d haveKeyguard=%d"
                , mSystemBooted, mShowingBootMessages, haveBootMsg, haveApp
                , haveWallpaper, wallpaperEnabled, haveKeyguard);
    }

    // If we are turning on the screen to show the boot message,
    // don't do it until the boot message is actually displayed.
    if (!mSystemBooted && !haveBootMsg) {
        return TRUE;
    }

    // If we are turning on the screen after the boot is completed
    // normally, don't do so until we have the application and
    // wallpaper.
    if (mSystemBooted && ((!haveApp && !haveKeyguard) || (wallpaperEnabled && !haveWallpaper))) {
        return TRUE;
    }

    return FALSE;
}

void CWindowManagerService::PerformEnableScreen()
{
    synchronized (mWindowMapLock) {
        if (DEBUG_BOOT) {
            Slogger::I(TAG, "performEnableScreen: mDisplayEnabled=%d mForceDisplayEnabled=%d mShowingBootMessages=%d mSystemBooted=%d mOnlyCore=%d"
                    , mDisplayEnabled, mForceDisplayEnabled, mShowingBootMessages, mSystemBooted, mOnlyCore);
                /*new RuntimeException("here").fillInStackTrace());*/
        }
        if (mDisplayEnabled) {
            return;
        }
        if (!mSystemBooted && !mShowingBootMessages) {
            return;
        }

        // Don't enable the screen until all existing windows have been drawn.
        if (!mForceDisplayEnabled && CheckWaitingForWindowsLocked()) {
            return;
        }

        if (!mBootAnimationStopped) {
            // Do this one time.
            // try {
            android::sp<android::IServiceManager> sm = android::defaultServiceManager();
            android::sp<android::IBinder> surfaceFlinger = sm->getService(android::String16("SurfaceFlinger"));

            if (surfaceFlinger != NULL) {
                Slogger::I(TAG, "******* TELLING SURFACE FLINGER WE ARE BOOTED!");
                android::Parcel data;
                data.writeInterfaceToken(android::String16("android.ui.ISurfaceComposer"));
                surfaceFlinger->transact(android::IBinder::FIRST_CALL_TRANSACTION, // BOOT_FINISHED
                                        data, NULL, 0);
            }
            // } catch (RemoteException ex) {
            //     Slog.e(TAG, "Boot completed: SurfaceFlinger is dead!");
            // }
            mBootAnimationStopped = TRUE;
        }

        if (!mForceDisplayEnabled && !CheckBootAnimationCompleteLocked()) {
            if (DEBUG_BOOT) Slogger::I(TAG, "performEnableScreen: Waiting for anim complete");
            return;
        }

        mDisplayEnabled = TRUE;
        if (DEBUG_SCREEN_ON || DEBUG_BOOT) Slogger::I(TAG, "******************** ENABLING SCREEN!");

        // Enable input dispatch.
        mInputMonitor->SetEventDispatchingLw(mEventDispatchingEnabled);
    }

    // try {
    mActivityManager->BootAnimationComplete();
    // } catch (RemoteException e) {
    // }

    mPolicy->EnableScreenAfterBoot();

    // Make sure the last requested orientation has been applied.
    UpdateRotationUnchecked(FALSE, FALSE);
}

Boolean CWindowManagerService::CheckBootAnimationCompleteLocked()
{
    AutoPtr<Elastos::Droid::Os::ISystemService> systemService;
    CSystemService::AcquireSingleton((Elastos::Droid::Os::ISystemService**)&systemService);
    Boolean isRunning;
    if (systemService->IsRunning(BOOT_ANIMATION_SERVICE, &isRunning), isRunning) {
        mH->RemoveMessages(H::CHECK_IF_BOOT_ANIMATION_FINISHED);
        Boolean result;
        mH->SendEmptyMessageDelayed(H::CHECK_IF_BOOT_ANIMATION_FINISHED,
                BOOT_ANIMATION_POLL_INTERVAL, &result);
        if (DEBUG_BOOT) Slogger::I(TAG, "checkBootAnimationComplete: Waiting for anim complete");
        return FALSE;
    }
    if (DEBUG_BOOT) Slogger::I(TAG, "checkBootAnimationComplete: Animation complete!");
    return TRUE;
}

void CWindowManagerService::ShowBootMessage(
    /* [in] */ ICharSequence* msg,
    /* [in] */ Boolean always)
{
    Boolean first = FALSE;

    synchronized (mWindowMapLock) {
        // if (DEBUG_BOOT) {
        //     RuntimeException here = new RuntimeException("here");
        //     here.fillInStackTrace();
        //     Slog.i(TAG, "showBootMessage: msg=" + msg + " always=" + always
        //             + " mAllowBootMessages=" + mAllowBootMessages
        //             + " mShowingBootMessages=" + mShowingBootMessages
        //             + " mSystemBooted=" + mSystemBooted, here);
        // }
        if (!mAllowBootMessages) {
            return;
        }
        if (!mShowingBootMessages) {
            if (!always) {
                return;
            }
            first = TRUE;
        }
        if (mSystemBooted) {
            return;
        }
        mShowingBootMessages = TRUE;
        mPolicy->ShowBootMessage(msg, always);
    }
    if (first) {
        PerformEnableScreen();
    }
}

void CWindowManagerService::HideBootMessagesLocked()
{
    // if (DEBUG_BOOT) {
    //     RuntimeException here = new RuntimeException("here");
    //     here.fillInStackTrace();
    //     Slog.i(TAG, "hideBootMessagesLocked: mDisplayEnabled=" + mDisplayEnabled
    //             + " mForceDisplayEnabled=" + mForceDisplayEnabled
    //             + " mShowingBootMessages=" + mShowingBootMessages
    //             + " mSystemBooted=" + mSystemBooted, here);
    // }
    if (mShowingBootMessages) {
        mShowingBootMessages = FALSE;
        mPolicy->HideBootMessages();
    }
}

ECode CWindowManagerService::SetInTouchMode(
    /* [in] */ Boolean mode)
{
    synchronized (mWindowMapLock) {
        mInTouchMode = mode;
    }
    return NOERROR;
}

void CWindowManagerService::ShowCircularDisplayMaskIfNeeded()
{
    // we're fullscreen and not hosted in an ActivityView
    AutoPtr<IResources> res;
    mContext->GetResources((IResources**)&res);
    Boolean value1, value2;
    if ((res->GetBoolean(Elastos::Droid::R::bool_::config_windowIsRound, &value1), value1)
            && (res->GetBoolean(Elastos::Droid::R::bool_::config_windowShowCircularMask, &value2), value2)) {
        AutoPtr<IMessage> msg;
        mH->ObtainMessage(H::SHOW_CIRCULAR_DISPLAY_MASK, (IMessage**)&msg);
        Boolean result;
        mH->SendMessage(msg, &result);
    }
}

void CWindowManagerService::ShowEmulatorDisplayOverlayIfNeeded()
{
    AutoPtr<IResources> res;
    mContext->GetResources((IResources**)&res);
    AutoPtr<ISystemProperties> sysProp;
    CSystemProperties::AcquireSingleton((ISystemProperties**)&sysProp);
    Boolean value1, value2;
    if ((res->GetBoolean(Elastos::Droid::R::bool_::config_windowEnableCircularEmulatorDisplayOverlay, &value1), value1)
            && (sysProp->GetBoolean(PROPERTY_EMULATOR_CIRCULAR, FALSE, &value2), value2)
            && Build::HARDWARE.Contains(String("goldfish"))) {
        AutoPtr<IMessage> msg;
        mH->ObtainMessage(H::SHOW_EMULATOR_DISPLAY_OVERLAY, (IMessage**)&msg);
        Boolean result;
        mH->SendMessage(msg, &result);
    }
}

void CWindowManagerService::ShowCircularMask()
{
    synchronized (mWindowMapLock) {

        if (SHOW_LIGHT_TRANSACTIONS) Slogger::I(TAG, ">>> OPEN TRANSACTION showCircularMask");
        AutoPtr<ISurfaceControlHelper> scHelper;
        CSurfaceControlHelper::AcquireSingleton((ISurfaceControlHelper**)&scHelper);
        scHelper->OpenTransaction();
        // try {
        // TODO(multi-display): support multiple displays
        if (mCircularDisplayMask == NULL) {
            AutoPtr<IResources> res;
            mContext->GetResources((IResources**)&res);
            Int32 screenOffset;
            res->GetDimensionPixelSize(Elastos::Droid::R::dimen::circular_display_mask_offset, &screenOffset);

            Int32 layer;
            mPolicy->WindowTypeToLayerLw(IWindowManagerLayoutParams::TYPE_POINTER, &layer);
            Int32 zOrder = layer * TYPE_LAYER_MULTIPLIER + 10;
            AutoPtr<IDisplay> d = GetDefaultDisplayContentLocked()->GetDisplay();
            mCircularDisplayMask = new CircularDisplayMask(d, mFxSession, zOrder, screenOffset);
        }
        mCircularDisplayMask->SetVisibility(TRUE);
        // } finally {
        //     SurfaceControl.closeTransaction();
        //     if (SHOW_LIGHT_TRANSACTIONS) Slog.i(TAG,
        //             "<<< CLOSE TRANSACTION showCircularMask");
        // }
        scHelper->CloseTransaction();
        if (SHOW_LIGHT_TRANSACTIONS) Slogger::I(TAG, "<<< CLOSE TRANSACTION showCircularMask");
    }
}

void CWindowManagerService::ShowEmulatorDisplayOverlay()
{
    synchronized (mWindowMapLock) {

        if (SHOW_LIGHT_TRANSACTIONS) Slogger::I(TAG, ">>> OPEN TRANSACTION showEmulatorDisplayOverlay");
        AutoPtr<ISurfaceControlHelper> scHelper;
        CSurfaceControlHelper::AcquireSingleton((ISurfaceControlHelper**)&scHelper);
        scHelper->OpenTransaction();
        // try {
        if (mEmulatorDisplayOverlay == NULL) {
            Int32 layer;
            mPolicy->WindowTypeToLayerLw(IWindowManagerLayoutParams::TYPE_POINTER, &layer);
            Int32 zOrder = layer * TYPE_LAYER_MULTIPLIER + 10;
            AutoPtr<IDisplay> d = GetDefaultDisplayContentLocked()->GetDisplay();
            mEmulatorDisplayOverlay = new EmulatorDisplayOverlay(mContext, d,
                    mFxSession, zOrder);
        }
        mEmulatorDisplayOverlay->SetVisibility(TRUE);
        // } finally {
        //     SurfaceControl.closeTransaction();
        //     if (SHOW_LIGHT_TRANSACTIONS) Slog.i(TAG,
        //             "<<< CLOSE TRANSACTION showEmulatorDisplayOverlay");
        // }
        scHelper->CloseTransaction();
        if (SHOW_LIGHT_TRANSACTIONS) Slogger::I(TAG, "<<< CLOSE TRANSACTION showEmulatorDisplayOverlay");
    }
}

ECode CWindowManagerService::ShowStrictModeViolation(
    /* [in] */ Boolean on)
{
    Int32 pid = Binder::GetCallingPid();
    AutoPtr<IMessage> msg;
    mH->ObtainMessage(H::SHOW_STRICT_MODE_VIOLATION, on ? 1 : 0, pid, (IMessage**)&msg);

    Boolean result;
    return mH->SendMessage(msg, &result);
}

void CWindowManagerService::ShowStrictModeViolation(
    /* [in] */ Int32 arg,
    /* [in] */ Int32 pid)
{
    Boolean on = arg != 0;
    AutoLock lock(mWindowMapLock);
    // Ignoring requests to enable the red border from clients
    // which aren't on screen.  (e.g. Broadcast Receivers in
    // the background..)
    if (on) {
        Boolean isVisible = FALSE;
        Int32 numDisplays;
        mDisplayContents->GetSize(&numDisplays);
        for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
            AutoPtr<IInterface> value;
            mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
            AutoPtr<DisplayContent> dc = (DisplayContent*)(IObject*)value.Get();
            AutoPtr<WindowList> windows = dc->GetWindowList();
            WindowList::Iterator it = windows->Begin();
            for (; it != windows->End(); ++it) {
                AutoPtr<WindowState> ws = *it;
                Boolean isVisible;
                if (ws->mSession->mPid == pid && (ws->IsVisibleLw(&isVisible), isVisible)) {
                    isVisible = TRUE;
                    break;
                }
            }
        }
        if (!isVisible) {
            return;
        }
    }

    // if (SHOW_LIGHT_TRANSACTIONS) Slog.i(TAG,
    //         ">>> OPEN TRANSACTION showStrictModeViolation");
    AutoPtr<ISurfaceControlHelper> surfaceHelper;
    CSurfaceControlHelper::AcquireSingleton((ISurfaceControlHelper**)&surfaceHelper);
    surfaceHelper->OpenTransaction();
    // try {
    // TODO(multi-display): support multiple displays
    if (mStrictModeFlash == NULL) {
        AutoPtr<IDisplay> d = GetDefaultDisplayContentLocked()->GetDisplay();
        mStrictModeFlash = new StrictModeFlash(d, mFxSession);
    }
    mStrictModeFlash->SetVisibility(on);
    surfaceHelper->CloseTransaction();
    // } finally {
    surfaceHelper->CloseTransaction();
    if (SHOW_LIGHT_TRANSACTIONS) Slogger::I(TAG,
            "<<< CLOSE TRANSACTION showStrictModeViolation");
    // }
}

ECode CWindowManagerService::SetStrictModeVisualIndicatorPreference(
    /* [in] */ const String& value)
{
    AutoPtr<ISystemProperties> sysProp;
    CSystemProperties::AcquireSingleton((ISystemProperties**)&sysProp);
    sysProp->Set(String("persist.sys.strictmode.visual")/*TODO: StrictMode::VISUAL_PROPERTY*/, value);
    return NOERROR;
}

void CWindowManagerService::ConvertCropForSurfaceFlinger(
    /* [in] */ IRect* crop,
    /* [in] */ Int32 rot,
    /* [in] */ Int32 dw,
    /* [in] */ Int32 dh)
{
    if (rot == ISurface::ROTATION_90) {
        Int32 tmp, right, bottom, left;
        crop->GetTop(&tmp);
        crop->GetRight(&right);
        crop->SetTop(dw - right);
        crop->GetBottom(&bottom);
        crop->SetRight(bottom);
        crop->GetLeft(&left);
        crop->SetBottom(dw - left);
        crop->SetLeft(tmp);
    }
    else if (rot == ISurface::ROTATION_180) {
        Int32 tmp, bottom, left;
        crop->GetTop(&tmp);
        crop->GetBottom(&bottom);
        crop->SetTop(dh - bottom);
        crop->SetBottom(dh - tmp);
        crop->GetRight(&tmp);
        crop->GetLeft(&left);
        crop->SetRight(dw - left);
        crop->SetLeft(dw - tmp);
    }
    else if (rot == ISurface::ROTATION_270) {
        Int32 tmp, right, bottom, left;
        crop->GetTop(&tmp);
        crop->GetLeft(&left);
        crop->SetTop(left);
        crop->GetBottom(&bottom);
        crop->SetLeft(dh - bottom);
        crop->GetRight(&right);
        crop->SetBottom(right);
        crop->SetRight(dh - tmp);
    }
}

ECode CWindowManagerService::ScreenshotApplications(
    /* [in] */ IBinder* appToken,
    /* [in] */ Int32 displayId,
    /* [in] */ Int32 width,
    /* [in] */ Int32 height,
    /* [in] */ Boolean force565,
    /* [out] */ IBitmap** bitmap)
{
    VALIDATE_NOT_NULL(bitmap)
    *bitmap = NULL;

    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::READ_FRAME_BUFFER,
            String("ScreenshotApplications()"))) {
        Slogger::E(TAG, "Requires READ_FRAME_BUFFER permission");
        return E_SECURITY_EXCEPTION;
        // throw new SecurityException("Requires READ_FRAME_BUFFER permission");
    }

    AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
    if (displayContent == NULL) {
        if (DEBUG_SCREENSHOT) Slogger::I(TAG, "Screenshot of %p: returning null. No Display for displayId=%d"
                , appToken, displayId);
        *bitmap = NULL;
        return NOERROR;
    }
    AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
    Int32 dw, dh;
    displayInfo->GetLogicalWidth(&dw);
    displayInfo->GetLogicalHeight(&dh);
    if (dw == 0 || dh == 0) {
        if (DEBUG_SCREENSHOT) Slogger::I(TAG, "Screenshot of %p: returning null. logical widthxheight=%dx%d"
                , appToken, dw, dh);
        *bitmap = NULL;
        return NOERROR;
    }

    AutoPtr<IBitmap> bm;

    Int32 maxLayer = 0;
    AutoPtr<IRect> frame, stackBounds;
    CRect::New((IRect**)&frame);
    CRect::New((IRect**)&stackBounds);

    Int32 rot = ISurface::ROTATION_0;

    Boolean screenshotReady;
    Int32 minLayer;
    if (appToken == NULL) {
        screenshotReady = TRUE;
        minLayer = 0;
    }
    else {
        screenshotReady = FALSE;
        minLayer = Elastos::Core::Math::INT32_MAX_VALUE;
    }

    Int32 retryCount = 0;
    AutoPtr<WindowState> appWin;

    Boolean appIsImTarget = mInputMethodTarget != NULL
            && mInputMethodTarget->mAppToken != NULL
            && mInputMethodTarget->mAppToken->mAppToken != NULL
            && IBinder::Probe(mInputMethodTarget->mAppToken->mAppToken) == appToken;

    Int32 layer;
    mPolicy->WindowTypeToLayerLw(IWindowManagerLayoutParams::TYPE_APPLICATION, &layer);
    Int32 aboveAppLayer = (layer + 1) * TYPE_LAYER_MULTIPLIER + TYPE_LAYER_OFFSET;

    while (TRUE) {
        if (retryCount++ > 0) {
            // try {
            Thread::Sleep(100);
            // } catch (InterruptedException e) {
            // }
        }
        synchronized (mWindowMapLock) {
            // Figure out the part of the screen that is actually the app.
            appWin = NULL;
            AutoPtr<WindowList> windows = displayContent->GetWindowList();
            WindowList::ReverseIterator rit = windows->RBegin();
            for (; rit != windows->REnd(); ++rit) {
                AutoPtr<WindowState> ws = *rit;
                if (!ws->mHasSurface) {
                    continue;
                }
                if (ws->mLayer >= aboveAppLayer) {
                    continue;
                }
                if (ws->mIsImWindow) {
                    if (!appIsImTarget) {
                        continue;
                    }
                }
                else if (ws->mIsWallpaper) {
                    // Fall through.
                }
                else if (appToken != NULL) {
                    if (ws->mAppToken == NULL || ws->mAppToken->mToken.Get() != appToken) {
                        // This app window is of no interest if it is not associated with the
                        // screenshot app.
                        continue;
                    }
                    appWin = ws;
                }

                // Include this window.

                AutoPtr<WindowStateAnimator> winAnim = ws->mWinAnimator;
                if (maxLayer < winAnim->mSurfaceLayer) {
                    maxLayer = winAnim->mSurfaceLayer;
                }
                if (minLayer > winAnim->mSurfaceLayer) {
                    minLayer = winAnim->mSurfaceLayer;
                }

                // Don't include wallpaper in bounds calculation
                if (!ws->mIsWallpaper) {
                    AutoPtr<IRect> wf = ws->mFrame;
                    AutoPtr<IRect> cr = ws->mContentInsets;
                    Int32 wfLeft, crLeft;
                    wf->GetLeft(&wfLeft);
                    cr->GetLeft(&crLeft);
                    Int32 left = wfLeft + crLeft;
                    Int32 wfTop, crTop;
                    wf->GetTop(&wfTop);
                    cr->GetTop(&crTop);
                    Int32 top = wfTop + crTop;
                    Int32 wfRight, crRight;
                    wf->GetRight(&wfRight);
                    cr->GetRight(&crRight);
                    Int32 right = wfRight - crRight;
                    Int32 wfBottom, crBottom;
                    wf->GetBottom(&wfBottom);
                    cr->GetBottom(&crBottom);
                    Int32 bottom = wfBottom - crBottom;
                    frame->Union(left, top, right, bottom);
                    ws->GetStackBounds(stackBounds);
                    Boolean result;
                    frame->Intersect(stackBounds, &result);
                }

                Boolean isDisplayed;
                if (ws->mAppToken != NULL && ws->mAppToken->mToken.Get() == appToken
                        && (ws->IsDisplayedLw(&isDisplayed), isDisplayed)) {
                    screenshotReady = TRUE;
                }
            }

            if (appToken != NULL && appWin == NULL) {
                // Can't find a window to snapshot.
                if (DEBUG_SCREENSHOT) Slogger::I(TAG,
                        "Screenshot: Couldn't find a surface matching %p", appToken);
                *bitmap = NULL;
                return NOERROR;
            }

            if (!screenshotReady) {
                if (retryCount > MAX_SCREENSHOT_RETRIES) {
                    String str;
                    if (appWin) {
                        str = String("null");
                    }
                    else {
                        String winS;
                        appWin->ToString(&winS);
                        str = winS + " drawState=" + StringUtils::ToString(appWin->mWinAnimator->mDrawState);
                    }
                    Slogger::I(TAG, "Screenshot max retries %d of %p appWin=%s"
                            , retryCount, appToken, str.string());
                    *bitmap = NULL;
                    return NOERROR;
                }

                // Delay and hope that window gets drawn.
                if (DEBUG_SCREENSHOT) {
                    Slogger::I(TAG, "Screenshot: No image ready for %p, %p drawState=%d"
                            , appToken, appWin.Get(), appWin->mWinAnimator->mDrawState);
                }
                continue;
            }

            // Screenshot is ready to be taken. Everything from here below will continue
            // through the bottom of the loop and return a value. We only stay in the loop
            // because we don't want to release the mWindowMap lock until the screenshot is
            // taken.

            if (maxLayer == 0) {
                if (DEBUG_SCREENSHOT)
                    Slogger::I(TAG, "Screenshot of %p: returning null maxLayer=%d", appToken, maxLayer);
                *bitmap = NULL;
                return NOERROR;
            }

            Boolean result;
            // Constrain frame to the screen size.
            frame->Intersect(0, 0, dw, dh, &result);

            // Tell surface flinger what part of the image to crop. Take the top
            // right part of the application, and crop the larger dimension to fit.
            AutoPtr<IRect> crop;
            CRect::New(frame, (IRect**)&crop);
            Int32 w, h;
            frame->GetWidth(&w);
            frame->GetHeight(&h);
            if (width / (Float)w < height / (Float)h) {
                Int32 cropWidth = (Int32)((Float)width / (Float)height * h);
                Int32 l;
                crop->GetLeft(&l);
                crop->SetRight(l + cropWidth);
            }
            else {
                Int32 cropHeight = (Int32)((Float)height / (Float)width * w);
                Int32 t;
                crop->GetTop(&t);
                crop->SetBottom(t + cropHeight);
            }

            // The screenshot API does not apply the current screen rotation.
            AutoPtr<IDisplay> dis = GetDefaultDisplayContentLocked()->GetDisplay();
            dis->GetRotation(&rot);

            if (rot == ISurface::ROTATION_90 || rot == ISurface::ROTATION_270) {
                rot = (rot == ISurface::ROTATION_90) ? ISurface::ROTATION_270 : ISurface::ROTATION_90;
            }

            // Surfaceflinger is not aware of orientation, so convert our logical
            // crop to surfaceflinger's portrait orientation.
            ConvertCropForSurfaceFlinger(crop, rot, dw, dh);

            if (DEBUG_SCREENSHOT) {
                Slogger::I(TAG, "Screenshot: %dx%d from %d to %d appToken=%p"
                        , dw, dh, minLayer, maxLayer, appToken);
                WindowList::Iterator it = windows->Begin();
                for (; it != windows->End(); ++it) {
                    AutoPtr<WindowState> win = *it;
                    Slogger::I(TAG, "%p: %d animLayer=%d surfaceLayer=%d", win.Get(), win->mLayer
                            , win->mWinAnimator->mAnimLayer, win->mWinAnimator->mSurfaceLayer);
                }
            }

            AutoPtr<ScreenRotationAnimation> screenRotationAnimation =
                    mAnimator->GetScreenRotationAnimationLocked(IDisplay::DEFAULT_DISPLAY);
            Boolean inRotation = screenRotationAnimation != NULL &&
                    screenRotationAnimation->IsAnimating();
            if (DEBUG_SCREENSHOT && inRotation) Slogger::V(TAG, "Taking screenshot while rotating");

            AutoPtr<ISurfaceControlHelper> scHelper;
            CSurfaceControlHelper::AcquireSingleton((ISurfaceControlHelper**)&scHelper);
            bm = NULL;
            scHelper->Screenshot(crop, width, height, minLayer, maxLayer, inRotation, rot, (IBitmap**)&bm);
            if (bm == NULL) {
                Slogger::W(TAG, "Screenshot failure taking screenshot for (%dx%d) to layer %d", dw, dh, maxLayer);
                *bitmap = NULL;
                return NOERROR;
            }
        }

        break;
    }

    if (DEBUG_SCREENSHOT) {
        // TEST IF IT's ALL BLACK
        AutoPtr< ArrayOf<Int32> > buffer = ArrayOf<Int32>::Alloc(2);
        Int32 w, h;
        bm->GetWidth(&w);
        bm->GetHeight(&h);
        (*buffer)[0] = w;
        (*buffer)[1] = h;
        bm->GetPixels(buffer, 0, w, 0, 0, w, h);
        Boolean allBlack = TRUE;
        Int32 firstColor = (*buffer)[0];
        for (Int32 i = 0; i < buffer->GetLength(); i++) {
            if ((*buffer)[i] != firstColor) {
                allBlack = FALSE;
                break;
            }
        }
        if (allBlack) {
            Slogger::I(TAG, "Screenshot %p was monochrome(%d)! mSurfaceLayer=%s minLayer=%d maxLayer=%d"
                    , appWin.Get(), firstColor
                    , (appWin != NULL ? StringUtils::ToString(appWin->mWinAnimator->mSurfaceLayer).string() : "null")
                    , minLayer, maxLayer);
        }
    }

    // Copy the screenshot bitmap to another buffer so that the gralloc backed
    // bitmap will not have a long lifetime. Gralloc memory can be pinned or
    // duplicated and might have a higher cost than a skia backed buffer.
    AutoPtr<IBitmap> ret;
    BitmapConfig config;
    bm->GetConfig((BitmapConfig*)&config);
    bm->Copy(config, TRUE, (IBitmap**)&ret);
    bm->Recycle();
    *bitmap = ret;
    REFCOUNT_ADD(*bitmap)
    return NOERROR;
}

ECode CWindowManagerService::FreezeRotation(
    /* [in] */ Int32 rotation)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::SET_ORIENTATION,
            String("freezeRotation()"))) {
        Slogger::E(TAG, "Requires SET_ORIENTATION permission");
        return E_SECURITY_EXCEPTION;
        // throw new SecurityException("Requires SET_ORIENTATION permission");
    }
    if (rotation < -1 || rotation > ISurface::ROTATION_270) {
        Slogger::E(TAG, "Rotation argument must be -1 or a valid rotation constant.");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
        // throw new IllegalArgumentException("Rotation argument must be -1 or a valid "
        //         + "rotation constant.");
    }

    if (DEBUG_ORIENTATION) Slogger::V(TAG, "freezeRotation: mRotation=%d", mRotation);

    Int64 origId = Binder::ClearCallingIdentity();
    // try {
    mPolicy->SetUserRotationMode(IWindowManagerPolicy::USER_ROTATION_LOCKED,
            rotation == -1 ? mRotation : rotation);
    // } finally {
    //     Binder.restoreCallingIdentity(origId);
    // }
    Binder::RestoreCallingIdentity(origId);

    UpdateRotationUnchecked(FALSE, FALSE);

    return NOERROR;
}

ECode CWindowManagerService::ThawRotation()
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::SET_ORIENTATION,
            String("thawRotation()"))) {
        Slogger::E(TAG, "Requires SET_ORIENTATION permission");
        return E_SECURITY_EXCEPTION;
        // throw new SecurityException("Requires SET_ORIENTATION permission");
    }

    if (DEBUG_ORIENTATION) Slogger::V(TAG, "thawRotation: mRotation=%d", mRotation);

    Int64 origId = Binder::ClearCallingIdentity();
    // try {
    mPolicy->SetUserRotationMode(IWindowManagerPolicy::USER_ROTATION_FREE, 777); // rot not used
    // } finally {
    //     Binder.restoreCallingIdentity(origId);
    // }
    Binder::RestoreCallingIdentity(origId);

    UpdateRotationUnchecked(FALSE, FALSE);
    return NOERROR;
}

ECode CWindowManagerService::UpdateRotation(
    /* [in] */ Boolean alwaysSendConfiguration,
    /* [in] */ Boolean forceRelayout)
{
    UpdateRotationUnchecked(alwaysSendConfiguration, forceRelayout);
    return NOERROR;
}

void CWindowManagerService::PauseRotationLocked()
{
    mDeferredRotationPauseCount += 1;
}

void CWindowManagerService::ResumeRotationLocked()
{
    if (mDeferredRotationPauseCount > 0) {
        mDeferredRotationPauseCount -= 1;
        if (mDeferredRotationPauseCount == 0) {
            Boolean changed = UpdateRotationUncheckedLocked(FALSE);
            if (changed) {
                Boolean result;
                mH->SendEmptyMessage(H::SEND_NEW_CONFIGURATION, &result);
            }
        }
    }
}

void CWindowManagerService::UpdateRotationUnchecked(
    /* [in] */ Boolean alwaysSendConfiguration,
    /* [in] */ Boolean forceRelayout)
{
    // if(DEBUG_ORIENTATION) Slog.v(TAG, "updateRotationUnchecked("
    //            + "alwaysSendConfiguration=" + alwaysSendConfiguration + ")");

    Int64 origId = Binder::ClearCallingIdentity();
    Boolean changed;
    {
        AutoLock lock(mWindowMapLock);
        changed = UpdateRotationUncheckedLocked(FALSE);
        if (!changed || forceRelayout) {
            GetDefaultDisplayContentLocked()->mLayoutNeeded = TRUE;
            PerformLayoutAndPlaceSurfacesLocked();
        }
    }

    if (changed || alwaysSendConfiguration) {
        SendNewConfiguration();
    }

    Binder::RestoreCallingIdentity(origId);
}

Boolean CWindowManagerService::UpdateRotationUncheckedLocked(
    /* [in] */ Boolean inTransaction)
{
    if (mDeferredRotationPauseCount > 0) {
        // Rotation updates have been paused temporarily.  Defer the update until
        // updates have been resumed.
        // if (DEBUG_ORIENTATION) Slog.v(TAG, "Deferring rotation, rotation is paused.");
        return FALSE;
    }

    AutoPtr<ScreenRotationAnimation> screenRotationAnimation =
            mAnimator->GetScreenRotationAnimationLocked(IDisplay::DEFAULT_DISPLAY);
    if (screenRotationAnimation != NULL && screenRotationAnimation->IsAnimating()) {
        // Rotation updates cannot be performed while the previous rotation change
        // animation is still in progress.  Skip this update.  We will try updating
        // again after the animation is finished and the display is unfrozen.
        // if (DEBUG_ORIENTATION) Slog.v(TAG, "Deferring rotation, animation in progress.");
        return FALSE;
    }

    if (!mDisplayEnabled) {
        // No point choosing a rotation if the display is not enabled.
        // if (DEBUG_ORIENTATION) Slog.v(TAG, "Deferring rotation, display is not enabled.");
        return FALSE;
    }

    // TODO: Implement forced rotation changes.
    //       Set mAltOrientation to indicate that the application is receiving
    //       an orientation that has different metrics than it expected.
    //       eg. Portrait instead of Landscape.

    // todo: Eric
    Int32 rotation = ISurface::ROTATION_0;
    mPolicy->RotationForOrientationLw(mForcedAppOrientation, mRotation, &rotation);
    Boolean isCompatible;
    mPolicy->RotationHasCompatibleMetricsLw(
            mForcedAppOrientation, rotation, &isCompatible);
    Boolean altOrientation = !isCompatible;

    // if (DEBUG_ORIENTATION) {
    //     Slog.v(TAG, "Application requested orientation "
    //             + mForcedAppOrientation + ", got rotation " + rotation
    //             + " which has " + (altOrientation ? "incompatible" : "compatible")
    //             + " metrics");
    // }

    if (mRotation == rotation && mAltOrientation == altOrientation) {
        // No change.
        return FALSE;
    }

    // if (DEBUG_ORIENTATION) {
    //     Slog.v(TAG,
    //         "Rotation changed to " + rotation + (altOrientation ? " (alt)" : "")
    //         + " from " + mRotation + (mAltOrientation ? " (alt)" : "")
    //         + ", forceApp=" + mForcedAppOrientation);
    // }

    mRotation = rotation;
    mAltOrientation = altOrientation;
    mPolicy->SetRotationLw(mRotation);

    mWindowsFreezingScreen = TRUE;
    Boolean result;
    mH->SendEmptyMessageDelayed(H::WINDOW_FREEZE_TIMEOUT, WINDOW_FREEZE_TIMEOUT_DURATION, &result);
    mWaitingForConfig = TRUE;
    AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();
    displayContent->mLayoutNeeded = TRUE;
    AutoPtr< ArrayOf<Int32> > anim = ArrayOf<Int32>::Alloc(2);
    if (displayContent->IsDimming()) {
        (*anim)[0] = (*anim)[1] = 0;
    }
    else {
        mPolicy->SelectRotationAnimationLw(anim);
    }
    StartFreezingDisplayLocked(inTransaction, (*anim)[0], (*anim)[1]);
    // startFreezingDisplayLocked can reset the ScreenRotationAnimation.
    screenRotationAnimation =
            mAnimator->GetScreenRotationAnimationLocked(IDisplay::DEFAULT_DISPLAY);

    // We need to update our screen size information to match the new
    // rotation.  Note that this is redundant with the later call to
    // sendNewConfiguration() that must be called after this function
    // returns...  however we need to do the screen size part of that
    // before then so we have the correct size to use when initializing
    // the rotation animation for the new rotation.
    ComputeScreenConfigurationLocked(NULL);

    AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
    if (!inTransaction) {
        if (SHOW_TRANSACTIONS) {
            Slogger::I(TAG, ">>> OPEN TRANSACTION setRotationUnchecked");
        }
        AutoPtr<ISurfaceControlHelper> helper;
        CSurfaceControlHelper::AcquireSingleton((ISurfaceControlHelper**)&helper);
        helper->OpenTransaction();
    }
    // try {
    // NOTE: We disable the rotation in the emulator because
    //       it doesn't support hardware OpenGL emulation yet.
    if (CUSTOM_SCREEN_ROTATION && screenRotationAnimation != NULL
            && screenRotationAnimation->HasScreenshot()) {
        Int32 w, h;
        displayInfo->GetLogicalWidth(&w);
        displayInfo->GetLogicalHeight(&h);
        if (screenRotationAnimation->SetRotationInTransaction(
                rotation, mFxSession,
                MAX_ANIMATION_DURATION, GetTransitionAnimationScaleLocked(), w, h)) {
            ScheduleAnimationLocked();
        }
    }

    mDisplayManagerInternal->PerformTraversalInTransactionFromWindowManager();
    // } finally {
    if (!inTransaction) {
        AutoPtr<ISurfaceControlHelper> helper;
        CSurfaceControlHelper::AcquireSingleton((ISurfaceControlHelper**)&helper);
        helper->CloseTransaction();
        if (SHOW_LIGHT_TRANSACTIONS) {
            Slogger::I(TAG, "<<< CLOSE TRANSACTION setRotationUnchecked");
        }
    }
    // }

    AutoPtr<WindowList> windows = displayContent->GetWindowList();
    WindowList::ReverseIterator rit;
    for (rit = windows->RBegin(); rit != windows->REnd(); ++rit) {
        AutoPtr<WindowState> w = *rit;
        if (w->mHasSurface) {
            // if (DEBUG_ORIENTATION) Slog.v(TAG, "Set mOrientationChanging of " + w);
            w->mOrientationChanging = TRUE;
            mInnerFields->mOrientationChangeComplete = FALSE;
        }
        w->mLastFreezeDuration = 0;
    }

    List<AutoPtr<RotationWatcher> >::ReverseIterator watcherRIt
            = mRotationWatchers.RBegin();
    for (; watcherRIt != mRotationWatchers.REnd(); ++watcherRIt) {
        // try {
        (*watcherRIt)->mWatcher->OnRotationChanged(rotation);
        // } catch (RemoteException e) {
        // }
    }

    //TODO (multidisplay): Magnification is supported only for the default display.
    // Announce rotation only if we will not animate as we already have the
    // windows in final state. Otherwise, we make this call at the rotation end.
    if (screenRotationAnimation == NULL && mAccessibilityController != NULL
            && displayContent->GetDisplayId() == IDisplay::DEFAULT_DISPLAY) {
        mAccessibilityController->OnRotationChangedLocked(
                GetDefaultDisplayContentLocked(), rotation);
    }

    return TRUE;
}

ECode CWindowManagerService::GetRotation(
    /* [out] */ Int32* rotation)
{
    VALIDATE_NOT_NULL(rotation)
    *rotation = mRotation;
    return NOERROR;
}

ECode CWindowManagerService::IsRotationFrozen(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    Int32 mode;
    mPolicy->GetUserRotationMode(&mode);
    *result = mode == IWindowManagerPolicy::USER_ROTATION_LOCKED;
    return NOERROR;
}

ECode CWindowManagerService::WatchRotation(
    /* [in] */ IRotationWatcher* watcher,
    /* [out] */ Int32* rotation)
{
    VALIDATE_NOT_NULL(rotation)

    AutoPtr<IBinder> watcherBinder = IBinder::Probe(watcher);
    AutoPtr<RotationWatcherDeathRecipint> dr = new RotationWatcherDeathRecipint(this, watcherBinder);

    AutoLock lock(mWindowMapLock);

    AutoPtr<IProxy> proxy = (IProxy*)watcher->Probe(EIID_IProxy);
    assert(proxy != NULL);
    proxy->LinkToDeath(dr.Get(), 0);
    AutoPtr<RotationWatcher> w = new RotationWatcher(watcher, dr);
    mRotationWatchers.PushBack(w);

    *rotation = mRotation;
    return NOERROR;
}

ECode CWindowManagerService::RemoveRotationWatcher(
    /* [in] */ IRotationWatcher* watcher)
{
    AutoPtr<IBinder> watcherBinder = IBinder::Probe(watcher);
    synchronized (mWindowMapLock) {
        List< AutoPtr<RotationWatcher> >::Iterator it = mRotationWatchers.Begin();
        while (it != mRotationWatchers.End()) {
            AutoPtr<RotationWatcher> rotationWatcher = *it;
            AutoPtr<IBinder> binder = IBinder::Probe(rotationWatcher->mWatcher);
            if (watcherBinder == binder) {
                AutoPtr<IProxy> proxy = (IProxy*)IProxy::Probe(rotationWatcher->mWatcher);
                if (proxy != NULL) {
                    Boolean result;
                    proxy->UnlinkToDeath(rotationWatcher->mDeathRecipient, 0, &result);
                }
                it = mRotationWatchers.Erase(it);
            }
            else
                ++it;
        }
    }
    return NOERROR;
}

ECode CWindowManagerService::GetPreferredOptionsPanelGravity(
    /* [out] */ Int32* value)
{
    VALIDATE_NOT_NULL(value)

    AutoLock lock(mWindowMapLock);
    Int32 rotation;
    GetRotation(&rotation);

    // TODO(multidisplay): Assume that such devices physical keys are on the main screen.
    AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();
    if (displayContent->mInitialDisplayWidth < displayContent->mInitialDisplayHeight) {
        // On devices with a natural orientation of portrait
        switch (rotation) {
            default:
            case ISurface::ROTATION_0:
                *value = IGravity::CENTER_HORIZONTAL | IGravity::BOTTOM;
            case ISurface::ROTATION_90:
                *value = IGravity::RIGHT | IGravity::BOTTOM;
            case ISurface::ROTATION_180:
                *value = IGravity::CENTER_HORIZONTAL | IGravity::BOTTOM;
            case ISurface::ROTATION_270:
                *value = IGravity::START | IGravity::BOTTOM;
        }
    }

    // On devices with a natural orientation of landscape
    switch (rotation) {
        default:
        case ISurface::ROTATION_0:
            *value = IGravity::RIGHT | IGravity::BOTTOM;
        case ISurface::ROTATION_90:
            *value = IGravity::CENTER_HORIZONTAL | IGravity::BOTTOM;
        case ISurface::ROTATION_180:
            *value = IGravity::START | IGravity::BOTTOM;
        case ISurface::ROTATION_270:
            *value = IGravity::CENTER_HORIZONTAL | IGravity::BOTTOM;
    }

    return NOERROR;
}

ECode CWindowManagerService::StartViewServer(
    /* [in] */ Int32 port,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    if (IsSystemSecure()) {
        *result = FALSE;
        return NOERROR;
    }

    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::DUMP, String("startViewServer"))) {
        *result = FALSE;
        return NOERROR;
    }

    if (port < 1024) {
        *result = FALSE;
        return NOERROR;
    }

    if (mViewServer != NULL) {
        if (!mViewServer->IsRunning()) {
            // try {
            ECode ec = mViewServer->Start(result);
            if (FAILED(ec)) {
                Slogger::W(TAG, "View server did not start");
                *result = FALSE;
                return ec;
            }
            return NOERROR;
            // } catch (IOException e) {
            //     Slog.w(TAG, "View server did not start");
            // }
        }
    }

    // try {
    mViewServer = new ViewServer(this, port);
    ECode ec = mViewServer->Start(result);
    if (FAILED(ec)) {
        Slogger::W(TAG, "View server did not start");
        *result = FALSE;
        return ec;
    }
    // } catch (IOException e) {
    //     Slog.w(TAG, "View server did not start");
    // }
    return NOERROR;
}

Boolean CWindowManagerService::IsSystemSecure()
{
    AutoPtr<ISystemProperties> sysProp;
    CSystemProperties::AcquireSingleton((ISystemProperties**)&sysProp);
    String value1, value2;
    return (sysProp->Get(SYSTEM_SECURE, String("1"), &value1), String("1").Equals(value1)) &&
            (sysProp->Get(SYSTEM_DEBUGGABLE, String("0"), &value2), String("0").Equals(value2));
}

ECode CWindowManagerService::StopViewServer(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    *result = FALSE;

    if (IsSystemSecure()) {
        return NOERROR;
    }

    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::DUMP, String("stopViewServer"))) {
        return NOERROR;
    }

    if (mViewServer != NULL) {
        *result = mViewServer->Stop();
    }
    return NOERROR;
}

ECode CWindowManagerService::IsViewServerRunning(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    if (IsSystemSecure()) {
        *result = FALSE;
        return NOERROR;
    }

    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::DUMP, String("isViewServerRunning"))) {
        *result = FALSE;
        return NOERROR;
    }

    *result = mViewServer != NULL && mViewServer->IsRunning();
    return NOERROR;
}

Boolean CWindowManagerService::ViewServerListWindows(
    /* [in] */ ISocket* client)
{
    if (IsSystemSecure()) {
        return FALSE;
    }

    Boolean result = TRUE;

    WindowList windows;

    synchronized (mWindowMapLock) {
        //noinspection unchecked
        Int32 numDisplays;
        mDisplayContents->GetSize(&numDisplays);
        for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
            AutoPtr<IInterface> value;
            mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
            AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
            AutoPtr<WindowList> wins = displayContent->GetWindowList();
            WindowList::Iterator it = wins->Begin();
            for (; it != wins->End(); ++it) {
                windows.PushBack(*it);
            }
        }
    }

    AutoPtr<IBufferedWriter> out;

    // Any uncaught exception will crash the system process
    // try {
    AutoPtr<IOutputStream> clientStream;
    if (FAILED(client->GetOutputStream((IOutputStream**)&clientStream))) {
        return FALSE;
    }
    AutoPtr<IOutputStreamWriter> osWriter;
    if (FAILED(COutputStreamWriter::New(clientStream, (IOutputStreamWriter**)&osWriter))) {
        return FALSE;
    }
    if (FAILED(CBufferedWriter::New(IWriter::Probe(osWriter), 8 * 1024, (IBufferedWriter**)&out))) {
        return FALSE;
    }

    AutoPtr<IWriter> outWriter = IWriter::Probe(out);

    WindowList::Iterator winIt = windows.Begin();
    for (; winIt != windows.End(); ++winIt) {
        AutoPtr<WindowState> w = *winIt;
        AutoPtr<ISystem> sys;
        CSystem::AcquireSingleton((ISystem**)&sys);
        Int32 hash;
        sys->IdentityHashCode((IInterface*)(IWindowState*)w.Get(), &hash);
        outWriter->Write(StringUtils::ToHexString(hash));
        outWriter->Write(String(" "));
        AutoPtr<ICharSequence> title;
        w->mAttrs->GetTitle((ICharSequence**)&title);
        IAppendable::Probe(out)->Append(title);
        outWriter->Write(String("\n"));
    }

    outWriter->Write(String("DONE.\n"));
    IFlushable::Probe(out)->Flush();
    // } catch (Exception e) {
    //     result = false;
    // } finally {
    //     if (out != null) {
    //         try {
    //             out.close();
    //         } catch (IOException e) {
    //             result = false;
    //         }
    //     }
    // }
    if (out != NULL) {
        // try {
        ICloseable::Probe(out)->Close();
        // } catch (IOException e) {
        //     result = false;
        // }
    }
    // } catch (IOException e) {
    //     result = false;
    // }

    return result;
}

Boolean CWindowManagerService::ViewServerGetFocusedWindow(
    /* [in] */ ISocket* client)
{
    if (IsSystemSecure()) {
        return FALSE;
    }

    AutoPtr<WindowState> focusedWindow = GetFocusedWindow();

    AutoPtr<IBufferedWriter> out;

    // Any uncaught exception will crash the system process
    // try {
    AutoPtr<IOutputStream> clientStream;
    client->GetOutputStream((IOutputStream**)&clientStream);
    AutoPtr<IOutputStreamWriter> osWriter;
    COutputStreamWriter::New(clientStream, (IOutputStreamWriter**)&osWriter);
    CBufferedWriter::New(IWriter::Probe(osWriter), 8 * 1024, (IBufferedWriter**)&out);

    AutoPtr<IWriter> outWriter = IWriter::Probe(out);
    if(focusedWindow != NULL) {
        AutoPtr<ISystem> sys;
        CSystem::AcquireSingleton((ISystem**)&sys);
        Int32 hash;
        sys->IdentityHashCode((IWindowState*)focusedWindow.Get(), &hash);
        outWriter->Write(StringUtils::ToHexString(hash));
        outWriter->Write(' ');
        AutoPtr<ICharSequence> cs;
        focusedWindow->mAttrs->GetTitle((ICharSequence**)&cs);
        IAppendable::Probe(out)->Append(cs);
    }
    IWriter::Probe(out)->Write('\n');
    IFlushable::Probe(out)->Flush();
    // } catch (Exception e) {
    //     result = false;
    // } finally {
    //     if (out != null) {
    //         try {
    //             out.close();
    //         } catch (IOException e) {
    //             result = false;
    //         }
    //     }
    // }

    // try {
    ICloseable::Probe(out)->Close();
    return TRUE;
}

Boolean CWindowManagerService::ViewServerWindowCommand(
    /* [in] */ ISocket* client,
    /* [in] */ const String& command,
    /* [in] */ const String& param)
{
    if (IsSystemSecure()) {
        return FALSE;
    }

    String parameters(param);
    Boolean success = TRUE;
    AutoPtr<IParcel> data;
    AutoPtr<IParcel> reply;

    AutoPtr<IBufferedWriter> out;

    // Any uncaught exception will crash the system process
    // try {
    // Find the hashcode of the window
    Int32 index = parameters.IndexOf(' ');
    if (index == -1) {
        index = parameters.GetLength();
    }
    String code = parameters.Substring(0, index);
    Int32 hashCode = 0;
    // int hashCode = (int) Long.parseLong(code, 16);

    // Extract the command's parameter after the window description
    if (index < parameters.GetLength()) {
        parameters = parameters.Substring(index + 1);
    }
    else {
        parameters = String("");
    }

    AutoPtr<WindowState> window = FindWindow(hashCode);
    if (window == NULL) {
        return FALSE;
    }

    assert(0);
    // data = Parcel.obtain();
    // data.writeInterfaceToken("android.view.IWindow");
    // data->WriteString(command);
    // data->WriteString(parameters);
    // data->WriteInt32(1);
    // ParcelFileDescriptor.fromSocket(client).writeToParcel(data, 0);

    // reply = Parcel.obtain();

    AutoPtr<IBinder> binder = IBinder::Probe(window->mClient.Get());
    // TODO: GET THE TRANSACTION CODE IN A SAFER MANNER
    // binder.transact(IBinder.FIRST_CALL_TRANSACTION, data, reply, 0);

    // reply.readException();

    Boolean result;
    client->IsOutputShutdown(&result);
    if (!result) {
        AutoPtr<IOutputStream> clientStream;
        if (FAILED(client->GetOutputStream((IOutputStream**)&clientStream))) {
            success = FALSE;
        }
        AutoPtr<IOutputStreamWriter> osWriter;
        if (FAILED(COutputStreamWriter::New(clientStream, (IOutputStreamWriter**)&osWriter))) {
            success = FALSE;
        }
        if (FAILED(CBufferedWriter::New(IWriter::Probe(osWriter), (IBufferedWriter**)&out))) {
            success = FALSE;
        }
        IWriter::Probe(out)->Write(String("DONE\n"));
        IFlushable::Probe(out)->Flush();
    }

    // } catch (Exception e) {
    //     Slog.w(TAG, "Could not send command " + command + " with parameters " + parameters, e);
    //     success = false;
    // } finally {
    // if (data != NULL) {
    //     data->Recycle();
    // }
    // if (reply != NULL) {
    //     reply->Recycle();
    // }
    if (out != NULL) {
        // try {
        ICloseable::Probe(out)->Close();
        // } catch (IOException e) {

        // }
    //     }
    }

    return success;
}

void CWindowManagerService::AddWindowChangeListener(
    /* [in] */ IWindowChangeListener* listener)
{
    synchronized (mWindowMapLock) {
        mWindowChangeListeners.PushBack(listener);
    }
}

void CWindowManagerService::RemoveWindowChangeListener(
    /* [in] */ IWindowChangeListener* listener)
{
    synchronized (mWindowMapLock) {
        mWindowChangeListeners.Remove(listener);
    }
}

void CWindowManagerService::NotifyWindowsChanged()
{
    AutoPtr<ArrayOf<IWindowChangeListener*> > windowChangeListeners;

    synchronized (mWindowMapLock) {
        if (mWindowChangeListeners.IsEmpty())
            return;

        windowChangeListeners = ArrayOf<IWindowChangeListener*>::Alloc(mWindowChangeListeners.GetSize());
        List< AutoPtr<IWindowChangeListener> >::Iterator it = mWindowChangeListeners.Begin();
        for (Int32 i = 0; it != mWindowChangeListeners.End(); ++it, ++i) {
            windowChangeListeners->Set(i, *it);
        }
    }

    if (windowChangeListeners != NULL) {
        for (Int32 i = 0; i < windowChangeListeners->GetLength(); ++i) {
            (*windowChangeListeners)[i]->WindowsChanged();
        }
    }
}

void CWindowManagerService::NotifyFocusChanged()
{
    AutoPtr<ArrayOf<IWindowChangeListener*> > windowChangeListeners;

    synchronized (mWindowMapLock) {
        if (mWindowChangeListeners.IsEmpty())
            return;

        windowChangeListeners = ArrayOf<IWindowChangeListener*>::Alloc(mWindowChangeListeners.GetSize());
        List< AutoPtr<IWindowChangeListener> >::Iterator it = mWindowChangeListeners.Begin();
        for (Int32 i = 0; it != mWindowChangeListeners.End(); ++it, ++i) {
            windowChangeListeners->Set(i, *it);
        }
    }

    if (windowChangeListeners != NULL) {
        for (Int32 i = 0; i < windowChangeListeners->GetLength(); ++i) {
            (*windowChangeListeners)[i]->FocusChanged();
        }
    }
}

AutoPtr<WindowState> CWindowManagerService::FindWindow(
    /* [in] */ Int32 hashCode)
{
    if (hashCode == -1) {
        return GetFocusedWindow();
    }

    synchronized (mWindowMapLock) {
        Int32 numDisplays;
        mDisplayContents->GetSize(&numDisplays);
        for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
            AutoPtr<IInterface> value;
            mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
            AutoPtr<DisplayContent> dc = (DisplayContent*)(IObject*)value.Get();
            AutoPtr<WindowList> windows = dc->GetWindowList();
            WindowList::Iterator it = windows->Begin();
            for (; it != windows->End(); ++it) {
                AutoPtr<WindowState> w = *it;
                AutoPtr<ISystem> sys;
                CSystem::AcquireSingleton((ISystem**)&sys);
                Int32 hash;
                sys->IdentityHashCode((IInterface*)(IWindowState*)w.Get(), &hash);
                if (hash == hashCode) {
                    return w;
                }
            }
        }
    }

    return NULL;
}

ECode CWindowManagerService::SendNewConfiguration()
{
    // try {
    return mActivityManager->UpdateConfiguration(NULL);
    // } catch (RemoteException e) {
    // }
}

AutoPtr<IConfiguration> CWindowManagerService::ComputeNewConfiguration()
{
    AutoPtr<IConfiguration> config;
    synchronized (mWindowMapLock) {
        config = ComputeNewConfigurationLocked();
        if (config == NULL && mWaitingForConfig) {
            // Nothing changed but we are waiting for something... stop that!
            mWaitingForConfig = FALSE;
            AutoPtr<ICharSequence> cs;
            CString::New(String("new-config"), (ICharSequence**)&cs);
            mLastFinishedFreezeSource = IObject::Probe(cs);
            PerformLayoutAndPlaceSurfacesLocked();
        }
    }
    return config;
}

AutoPtr<IConfiguration> CWindowManagerService::ComputeNewConfigurationLocked()
{
    AutoPtr<IConfiguration> config;
    ASSERT_SUCCEEDED(CConfiguration::New((IConfiguration**)&config));
    config->SetFontScale(0);
    if (!ComputeScreenConfigurationLocked(config)) {
        return NULL;
    }
    return config;
}

void CWindowManagerService::AdjustDisplaySizeRanges(
    /* [in] */ IDisplayInfo* displayInfo,
    /* [in] */ Int32 rotation,
    /* [in] */ Int32 dw,
    /* [in] */ Int32 dh)
{
    // TODO: Multidisplay: for now only use with default display.
    Int32 width;
    mPolicy->GetConfigDisplayWidth(dw, dh, rotation, &width);
    Int32 smallAppW;
    displayInfo->GetSmallestNominalAppWidth(&smallAppW);
    if (width < smallAppW) {
        displayInfo->SetSmallestNominalAppWidth(width);
    }
    Int32 largeAppW;
    displayInfo->GetLargestNominalAppWidth(&largeAppW);
    if (width > largeAppW) {
        displayInfo->SetLargestNominalAppWidth(width);
    }
    Int32 height;
    mPolicy->GetConfigDisplayHeight(dw, dh, rotation, &height);
    Int32 smallAppH;
    displayInfo->GetSmallestNominalAppHeight(&smallAppH);
    if (height < smallAppH) {
        displayInfo->SetSmallestNominalAppHeight(height);
    }
    Int32 largeAppH;
    displayInfo->GetLargestNominalAppHeight(&largeAppH);
    if (height > largeAppH) {
        displayInfo->SetLargestNominalAppHeight(height);
    }
}

Int32 CWindowManagerService::ReduceConfigLayout(
    /* [in] */ Int32 curLayout,
    /* [in] */ Int32 rotation,
    /* [in] */ Float density,
    /* [in] */ Int32 dw,
    /* [in] */ Int32 dh)
{
    // TODO: Multidisplay: for now only use with default display.
    // Get the app screen size at this rotation.
    Int32 w;
    mPolicy->GetNonDecorDisplayWidth(dw, dh, rotation, &w);
    Int32 h;
    mPolicy->GetNonDecorDisplayHeight(dw, dh, rotation, &h);

    // Compute the screen layout size class for this rotation.
    Int32 longSize = w;
    Int32 shortSize = h;
    if (longSize < shortSize) {
        Int32 tmp = longSize;
        longSize = shortSize;
        shortSize = tmp;
    }
    longSize = (Int32)(longSize / density);
    shortSize = (Int32)(shortSize / density);
    AutoPtr<IConfigurationHelper> helper;
    CConfigurationHelper::AcquireSingleton((IConfigurationHelper**)&helper);
    Int32 result;
    helper->ReduceScreenLayout(curLayout, longSize, shortSize, &result);
    return result;
}

void CWindowManagerService::ComputeSizeRangesAndScreenLayout(
    /* [in] */ IDisplayInfo* displayInfo,
    /* [in] */ Boolean rotated,
    /* [in] */ Int32 dw,
    /* [in] */ Int32 dh,
    /* [in] */ Float density,
    /* [in] */ IConfiguration* outConfig)
{
    // TODO: Multidisplay: for now only use with default display.

    // We need to determine the smallest width that will occur under normal
    // operation.  To this, start with the base screen size and compute the
    // width under the different possible rotations.  We need to un-rotate
    // the current screen dimensions before doing this.
    Int32 unrotDw, unrotDh;
    if (rotated) {
        unrotDw = dh;
        unrotDh = dw;
    }
    else {
        unrotDw = dw;
        unrotDh = dh;
    }
    displayInfo->SetSmallestNominalAppWidth(1<<30);
    displayInfo->SetSmallestNominalAppHeight(1<<30);
    displayInfo->SetLargestNominalAppWidth(0);
    displayInfo->SetLargestNominalAppHeight(0);
    AdjustDisplaySizeRanges(displayInfo, ISurface::ROTATION_0, unrotDw, unrotDh);
    AdjustDisplaySizeRanges(displayInfo, ISurface::ROTATION_90, unrotDh, unrotDw);
    AdjustDisplaySizeRanges(displayInfo, ISurface::ROTATION_180, unrotDw, unrotDh);
    AdjustDisplaySizeRanges(displayInfo, ISurface::ROTATION_270, unrotDh, unrotDw);
    AutoPtr<IConfigurationHelper> helper;
    CConfigurationHelper::AcquireSingleton((IConfigurationHelper**)&helper);
    Int32 layout;
    outConfig->GetScreenLayout(&layout);
    Int32 sl;
    helper->ResetScreenLayout(layout, &sl);
    sl = ReduceConfigLayout(sl, ISurface::ROTATION_0, density, unrotDw, unrotDh);
    sl = ReduceConfigLayout(sl, ISurface::ROTATION_90, density, unrotDh, unrotDw);
    sl = ReduceConfigLayout(sl, ISurface::ROTATION_180, density, unrotDw, unrotDh);
    sl = ReduceConfigLayout(sl, ISurface::ROTATION_270, density, unrotDh, unrotDw);
    Int32 w;
    displayInfo->GetSmallestNominalAppWidth(&w);
    outConfig->SetSmallestScreenWidthDp((Int32)(w / density));
    outConfig->SetScreenLayout(sl);
}

Int32 CWindowManagerService::ReduceCompatConfigWidthSize(
    /* [in] */ Int32 curSize,
    /* [in] */ Int32 rotation,
    /* [in] */ IDisplayMetrics* dm,
    /* [in] */ Int32 dw,
    /* [in] */ Int32 dh)
{
    // TODO: Multidisplay: for now only use with default display.
    assert(dm != NULL);
    Int32 width;
    mPolicy->GetNonDecorDisplayWidth(dw, dh, rotation, &width);
    dm->SetNoncompatWidthPixels(width);
    Int32 height;
    mPolicy->GetNonDecorDisplayHeight(dw, dh, rotation, &height);
    dm->SetNoncompatHeightPixels(height);
    AutoPtr<ICompatibilityInfoHelper> helper;
    CCompatibilityInfoHelper::AcquireSingleton((ICompatibilityInfoHelper**)&helper);
    Float scale;
    helper->ComputeCompatibleScaling(dm, NULL, &scale);
    Float density;
    dm->GetDensity(&density);
    Int32 size = (Int32)(((width / scale) / density) + 0.5);
    if (curSize == 0 || size < curSize) {
        curSize = size;
    }
    return curSize;
}

Int32 CWindowManagerService::ComputeCompatSmallestWidth(
    /* [in] */ Boolean rotated,
    /* [in] */ IDisplayMetrics* dm,
    /* [in] */ Int32 dw,
    /* [in] */ Int32 dh)
{
    // TODO: Multidisplay: for now only use with default display.
    mTmpDisplayMetrics->SetTo(dm);
    AutoPtr<IDisplayMetrics> tmpDm = mTmpDisplayMetrics;
    Int32 unrotDw, unrotDh;
    if (rotated) {
        unrotDw = dh;
        unrotDh = dw;
    }
    else {
        unrotDw = dw;
        unrotDh = dh;
    }
    Int32 sw = ReduceCompatConfigWidthSize(0, ISurface::ROTATION_0, tmpDm, unrotDw, unrotDh);
    sw = ReduceCompatConfigWidthSize(sw, ISurface::ROTATION_90, tmpDm, unrotDh, unrotDw);
    sw = ReduceCompatConfigWidthSize(sw, ISurface::ROTATION_180, tmpDm, unrotDw, unrotDh);
    sw = ReduceCompatConfigWidthSize(sw, ISurface::ROTATION_270, tmpDm, unrotDh, unrotDw);
    return sw;
}

Boolean CWindowManagerService::ComputeScreenConfigurationLocked(
    /* [in] */ IConfiguration* config)
{
    if (!mDisplayReady) {
        return FALSE;
    }

    // TODO(multidisplay): For now, apply Configuration to main screen only.
    AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();

    // Use the effective "visual" dimensions based on current rotation
    Boolean rotated = (mRotation == ISurface::ROTATION_90
            || mRotation == ISurface::ROTATION_270);
    Int32 realdw = rotated ?
            displayContent->mBaseDisplayHeight : displayContent->mBaseDisplayWidth;
    Int32 realdh = rotated ?
            displayContent->mBaseDisplayWidth : displayContent->mBaseDisplayHeight;
    Int32 dw = realdw;
    Int32 dh = realdh;

    if (mAltOrientation) {
        if (realdw > realdh) {
            // Turn landscape into portrait.
            Int32 maxw = (Int32)(realdh / 1.3);
            if (maxw < realdw) {
                dw = maxw;
            }
        }
        else {
            // Turn portrait into landscape.
            Int32 maxh = (Int32)(realdw / 1.3);
            if (maxh < realdh) {
                dh = maxh;
            }
        }
    }

    if (config != NULL) {
        config->SetOrientation((dw <= dh) ? IConfiguration::ORIENTATION_PORTRAIT :
                IConfiguration::ORIENTATION_LANDSCAPE);
    }

    // Update application display metrics.
    Int32 appWidth;
    mPolicy->GetNonDecorDisplayWidth(dw, dh, mRotation, &appWidth);
    Int32 appHeight;
    mPolicy->GetNonDecorDisplayHeight(dw, dh, mRotation, &appHeight);
    AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
    {
        AutoLock lock(displayContent->mDisplaySizeLock);

        displayInfo->SetRotation(mRotation);
        displayInfo->SetLogicalWidth(dw);
        displayInfo->SetLogicalHeight(dh);
        displayInfo->SetLogicalDensityDpi(displayContent->mBaseDisplayDensity);
        displayInfo->SetAppWidth(appWidth);
        displayInfo->SetAppHeight(appHeight);
        AutoPtr<ICompatibilityInfoHelper> helper;
        CCompatibilityInfoHelper::AcquireSingleton((ICompatibilityInfoHelper**)&helper);
        AutoPtr<ICompatibilityInfo> defaultCompatibilityInfo;
        helper->GetDefault((ICompatibilityInfo**)&defaultCompatibilityInfo);
        displayInfo->GetLogicalMetrics(mRealDisplayMetrics, defaultCompatibilityInfo, NULL);
        displayInfo->GetAppMetrics(mDisplayMetrics);
        mDisplayManagerInternal->SetDisplayInfoOverrideFromWindowManager(
                displayContent->GetDisplayId(), displayInfo);
    }
    // if (false) {
    //     Slog.i(TAG, "Set app display size: " + appWidth + " x " + appHeight);
    // }

    AutoPtr<IDisplayMetrics> dm = mDisplayMetrics;
    AutoPtr<ICompatibilityInfoHelper> helper;
    CCompatibilityInfoHelper::AcquireSingleton((ICompatibilityInfoHelper**)&helper);
    helper->ComputeCompatibleScaling(dm,
            mCompatDisplayMetrics, &mCompatibleScreenScale);

    if (config != NULL) {
        Int32 disW, disH;
        mPolicy->GetConfigDisplayWidth(dw, dh, mRotation, &disW);
        mPolicy->GetConfigDisplayHeight(dw, dh, mRotation, &disH);
        Float density;
        dm->GetDensity(&density);
        config->SetScreenWidthDp((Int32)(disW / density));
        config->SetScreenHeightDp((Int32)(disH / density));
        ComputeSizeRangesAndScreenLayout(displayInfo, rotated, dw, dh, density, config);

        config->SetCompatScreenWidthDp((Int32)((disW / density) / mCompatibleScreenScale));
        config->SetCompatScreenHeightDp((Int32)((disH / density) / mCompatibleScreenScale));
        config->SetCompatSmallestScreenWidthDp(
                ComputeCompatSmallestWidth(rotated, dm, dw, dh));
        config->SetDensityDpi(displayContent->mBaseDisplayDensity);

        // Update the configuration based on available input devices, lid switch,
        // and platform configuration.
        config->SetTouchscreen(IConfiguration::TOUCHSCREEN_NOTOUCH);
        config->SetKeyboard(IConfiguration::KEYBOARD_NOKEYS);
        config->SetNavigation(IConfiguration::NAVIGATION_NONAV);

        Int32 keyboardPresence = 0;
        Int32 navigationPresence = 0;
        AutoPtr<ArrayOf<IInputDevice*> > devices;
        mInputManager->GetInputDevices((ArrayOf<IInputDevice*>**)&devices);
        Int32 len = devices->GetLength();
        for (Int32 i = 0; i < len; i++) {
            AutoPtr<IInputDevice> device = (*devices)[i];
            Boolean isVirtual;
            device->IsVirtual(&isVirtual);
            if (!isVirtual) {
                Int32 sources;
                device->GetSources(&sources);
                Boolean isExternal;
                device->IsExternal(&isExternal);
                Int32 presenceFlag =  isExternal?
                        IWindowManagerPolicy::PRESENCE_EXTERNAL :
                                IWindowManagerPolicy::PRESENCE_INTERNAL;

                if (mIsTouchDevice) {
                    if ((sources & IInputDevice::SOURCE_TOUCHSCREEN) ==
                            IInputDevice::SOURCE_TOUCHSCREEN) {
                        config->SetTouchscreen(IConfiguration::TOUCHSCREEN_FINGER);
                    }
                }
                else {
                    config->SetTouchscreen(IConfiguration::TOUCHSCREEN_NOTOUCH);
                }

                Int32 navigation;
                config->GetNavigation(&navigation);
                if ((sources & IInputDevice::SOURCE_TRACKBALL)
                            == IInputDevice::SOURCE_TRACKBALL) {
                    config->SetNavigation(IConfiguration::NAVIGATION_TRACKBALL);
                    navigationPresence |= presenceFlag;
                }
                else if ((sources & IInputDevice::SOURCE_DPAD) == IInputDevice::SOURCE_DPAD
                        && navigation == IConfiguration::NAVIGATION_NONAV) {
                    config->SetNavigation(IConfiguration::NAVIGATION_DPAD);
                    navigationPresence |= presenceFlag;
                }

                Int32 type;
                device->GetKeyboardType(&type);
                if (type == IInputDevice::KEYBOARD_TYPE_ALPHABETIC) {
                    config->SetKeyboard(IConfiguration::KEYBOARD_QWERTY);
                    keyboardPresence |= presenceFlag;
                }
            }
        }

        Int32 navigation;
        if ((config->GetNavigation(&navigation), navigation == IConfiguration::NAVIGATION_NONAV) && mHasPermanentDpad) {
            config->SetNavigation(IConfiguration::NAVIGATION_DPAD);
            navigationPresence |= IWindowManagerPolicy::PRESENCE_INTERNAL;
        }

        // Determine whether a hard keyboard is available and enabled.
        Int32 keyboard;
        config->GetKeyboard(&keyboard);
        Boolean hardKeyboardAvailable = keyboard != IConfiguration::KEYBOARD_NOKEYS;
        if (hardKeyboardAvailable != mHardKeyboardAvailable) {
            mHardKeyboardAvailable = hardKeyboardAvailable;
            mH->RemoveMessages(H::REPORT_HARD_KEYBOARD_STATUS_CHANGE);
            Boolean result;
            mH->SendEmptyMessage(H::REPORT_HARD_KEYBOARD_STATUS_CHANGE, &result);
        }
        if (mShowImeWithHardKeyboard) {
            config->SetKeyboard(IConfiguration::KEYBOARD_NOKEYS);
        }

        // Let the policy update hidden states.
        config->SetKeyboardHidden(IConfiguration::KEYBOARDHIDDEN_NO);
        config->SetHardKeyboardHidden(IConfiguration::HARDKEYBOARDHIDDEN_NO);
        config->SetNavigationHidden(IConfiguration::NAVIGATIONHIDDEN_NO);
        mPolicy->AdjustConfigurationLw(config, keyboardPresence, navigationPresence);
    }

    return TRUE;
}

Boolean CWindowManagerService::IsHardKeyboardAvailable()
{
    AutoLock lock(mWindowMapLock);
    return mHardKeyboardAvailable;
}

void CWindowManagerService::UpdateShowImeWithHardKeyboard()
{
    AutoPtr<IContentResolver> resolver;
    mContext->GetContentResolver((IContentResolver**)&resolver);
    Int32 value;
    Settings::Secure::GetInt32ForUser(
            resolver, ISettingsSecure::SHOW_IME_WITH_HARD_KEYBOARD, 0, mCurrentUserId, &value);
    Boolean showImeWithHardKeyboard = value == 1;
    synchronized (mWindowMapLock) {
        if (mShowImeWithHardKeyboard != showImeWithHardKeyboard) {
            mShowImeWithHardKeyboard = showImeWithHardKeyboard;
            Boolean result;
            mH->SendEmptyMessage(H::SEND_NEW_CONFIGURATION, &result);
        }
    }
}

void CWindowManagerService::SetOnHardKeyboardStatusChangeListener(
    /* [in] */ IOnHardKeyboardStatusChangeListener* listener)
{
    synchronized (mWindowMapLock) {
        mHardKeyboardStatusChangeListener = listener;
    }
}

void CWindowManagerService::NotifyHardKeyboardStatusChange()
{
    Boolean available;
    AutoPtr<IOnHardKeyboardStatusChangeListener> listener;
    synchronized (mWindowMapLock) {
        listener = mHardKeyboardStatusChangeListener;
        available = mHardKeyboardAvailable;
    }

    if (listener != NULL) {
        listener->OnHardKeyboardStatusChange(available);
    }
}

// -------------------------------------------------------------
// Drag and drop
// -------------------------------------------------------------

AutoPtr<IBinder> CWindowManagerService::PrepareDragSurface(
    /* [in] */ IIWindow* window,
    /* [in] */ ISurfaceSession* session,
    /* [in] */ Int32 flags,
    /* [in] */ Int32 width,
    /* [in] */ Int32 height,
    /* [in] */ ISurface* outSurface)
{
    // if (DEBUG_DRAG) {
    //     Slog.d(TAG, "prepare drag surface: w=" + width + " h=" + height
    //             + " flags=" + Integer.toHexString(flags) + " win=" + window
    //             + " asbinder=" + window.asBinder());
    // }

    // Int32 callerPid = Binder::GetCallingPid();
    Int64 origId = Binder::ClearCallingIdentity();
    AutoPtr<IBinder> token;

    // try {
    synchronized (mWindowMapLock) {
        // try {
        if (mDragState == NULL) {
            // TODO(multi-display): support other displays
            AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();
            AutoPtr<IDisplay> display = displayContent->GetDisplay();
            AutoPtr<ISurfaceControl> surface;
            ECode ec = CSurfaceControl::New(session, String("drag surface"), width, height,
                    IPixelFormat::TRANSLUCENT, ISurfaceControl::HIDDEN, (ISurfaceControl**)&surface);
            if (FAILED(ec)) {
                Slogger::E(TAG, "Can't allocate drag surface w=%d h=%d 0x%08x", width, height, ec);
                mDragState->Reset();
                mDragState = NULL;
                Binder::RestoreCallingIdentity(origId);
                return token;
            }
            Int32 stack;
            display->GetLayerStack(&stack);
            surface->SetLayerStack(stack);
            // if (SHOW_TRANSACTIONS) Slog.i(TAG, "  DRAG "
            //         + surface + ": CREATE");
            outSurface->CopyFrom(surface);
            AutoPtr<IBinder> winBinder = IBinder::Probe(window);
            CBinder::New((IBinder**)&token);
            mDragState = new DragState(this, token, surface, /*flags*/ 0, winBinder);
            token = NULL;
            CBinder::New((IBinder**)&token);
            mDragState->mToken = token;

            // 5 second timeout for this window to actually begin the drag
            AutoPtr<IInterface> obj = winBinder ? winBinder->Probe(EIID_IInterface) : NULL;
            mH->RemoveMessages(H::DRAG_START_TIMEOUT, obj);
            AutoPtr<IMessage> msg;
            mH->ObtainMessage(H::DRAG_START_TIMEOUT, obj, (IMessage**)&msg);
            Boolean result;
            mH->SendMessageDelayed(msg, 5000, &result);
        }
        else {
            Slogger::W(TAG, "Drag already in progress");
        }
    }
    // } catch (OutOfResourcesException e) {
    //     Slog.e(TAG, "Can't allocate drag surface w=" + width + " h=" + height, e);
    //     if (mDragState != null) {
    //         mDragState.reset();
    //         mDragState = null;
    //     }
    // }
    // } finally {
    //     Binder::RestoreCallingIdentity(origId);
    // }
    Binder::RestoreCallingIdentity(origId);

    return token;
}

// -------------------------------------------------------------
// Input Events and Focus Management
// -------------------------------------------------------------

ECode CWindowManagerService::PauseKeyDispatching(
    /* [in] */ IBinder* token)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("PauseKeyDispatching()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized (mWindowMapLock) {
        AutoPtr<WindowToken> wtoken;
        HashMap<AutoPtr<IBinder>, AutoPtr<WindowToken> >::Iterator it
                = mTokenMap.Find(token);
        if (it != mTokenMap.End()) {
            wtoken = it->mSecond;
        }
        if (wtoken != NULL) {
            mInputMonitor->PauseDispatchingLw(wtoken);
        }
    }

    return NOERROR;
}

ECode CWindowManagerService::ResumeKeyDispatching(
    /* [in] */ IBinder* token)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("ResumeKeyDispatching()"))) {
        // throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized (mWindowMapLock) {
        AutoPtr<WindowToken> wtoken;
        HashMap<AutoPtr<IBinder>, AutoPtr<WindowToken> >::Iterator it
                = mTokenMap.Find(token);
        if (it != mTokenMap.End()) {
            wtoken = it->mSecond;
        }
        if (wtoken != NULL) {
            mInputMonitor->ResumeDispatchingLw(wtoken);
        }
    }

    return NOERROR;
}

ECode CWindowManagerService::SetEventDispatching(
    /* [in] */ Boolean enabled)
{
    if (!CheckCallingPermission(
            Elastos::Droid::Manifest::permission::MANAGE_APP_TOKENS,
            String("ResumeKeyDispatching()"))) {
        //throw new SecurityException("Requires MANAGE_APP_TOKENS permission");
        Slogger::E(TAG, "Requires MANAGE_APP_TOKENS permission");
        return E_SECURITY_EXCEPTION;
    }

    synchronized (mWindowMapLock) {
        mEventDispatchingEnabled = enabled;
        if (mDisplayEnabled) {
            mInputMonitor->SetEventDispatchingLw(enabled);
        }
    }

    return NOERROR;
}

AutoPtr<WindowState> CWindowManagerService::GetFocusedWindow()
{
    AutoLock lock(mWindowMapLock);
    return GetFocusedWindowLocked();
}

AutoPtr<WindowState> CWindowManagerService::GetFocusedWindowLocked()
{
    return mCurrentFocus;
}

Boolean CWindowManagerService::DetectSafeMode()
{
    if (!mInputMonitor->WaitForInputDevicesReady(
            INPUT_DEVICES_READY_FOR_SAFE_MODE_DETECTION_TIMEOUT_MILLIS)) {
        Slogger::W(TAG, "Devices still not ready after waiting %d milliseconds before attempting to detect safe mode."
               , INPUT_DEVICES_READY_FOR_SAFE_MODE_DETECTION_TIMEOUT_MILLIS);
    }

    Int32 menuState = mInputManager->GetKeyCodeState(
        -1, IInputDevice::SOURCE_ANY, IKeyEvent::KEYCODE_MENU);
    Int32 sState = mInputManager->GetKeyCodeState(
        -1, IInputDevice::SOURCE_ANY, IKeyEvent::KEYCODE_S);
    Int32 dpadState = mInputManager->GetKeyCodeState(
        -1, IInputDevice::SOURCE_DPAD, IKeyEvent::KEYCODE_DPAD_CENTER);
    Int32 trackballState = mInputManager->GetScanCodeState(
        -1, IInputDevice::SOURCE_TRACKBALL, CInputManagerService::BTN_MOUSE);
    Int32 volumeDownState = mInputManager->GetKeyCodeState(
        -1, IInputDevice::SOURCE_ANY, IKeyEvent::KEYCODE_VOLUME_DOWN);
    mSafeMode = menuState > 0 || sState > 0 || dpadState > 0 || trackballState > 0
            || volumeDownState > 0;

    AutoPtr<ISystemProperties> sysProp;
    CSystemProperties::AcquireSingleton((ISystemProperties**)&sysProp);
    Int32 value;
    if (sysProp->GetInt32(ShutdownThread::REBOOT_SAFEMODE_PROPERTY, 0, &value), value != 0) {
        mSafeMode = TRUE;
        sysProp->Set(ShutdownThread::REBOOT_SAFEMODE_PROPERTY, String(""));
    }

    if (mSafeMode) {
        Slogger::I(TAG, "SAFE MODE ENABLED (menu=%d s=%d dpad=%d trackball=%d)",
                menuState, sState, dpadState, trackballState);
    }
    else {
        Slogger::I(TAG, "SAFE MODE not enabled");
    }
    mPolicy->SetSafeMode(mSafeMode);
    return mSafeMode;
}

ECode CWindowManagerService::DisplayReady()
{
    //ActionsCode(author:wh, BUGFIXED:BUG00238382 init all displays which surfaceflinger decteced before system ready)
    //displayReady(Display.DEFAULT_DISPLAY);
    AutoPtr< ArrayOf<IDisplay*> > displays;
    mDisplayManager->GetDisplays((ArrayOf<IDisplay*>**)&displays);
    for (Int32 i = 0; i < displays->GetLength(); ++i) {
        AutoPtr<IDisplay> display = (*displays)[i];
        Int32 displayId;
        display->GetDisplayId(&displayId);
        DisplayReady(displayId);
    }
    synchronized (mWindowMapLock) {
        AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();
        ReadForcedDisplaySizeAndDensityLocked(displayContent);
        mDisplayReady = TRUE;
    }

    // try {
    mActivityManager->UpdateConfiguration(NULL);
    // } catch (RemoteException e) {
    // }

    synchronized (mWindowMapLock) {
        AutoPtr<IPackageManager> packageManager;
        mContext->GetPackageManager((IPackageManager**)&packageManager);
        packageManager->HasSystemFeature(IPackageManager::FEATURE_TOUCHSCREEN, &mIsTouchDevice);
        AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();
        ConfigureDisplayPolicyLocked(displayContent);
    }

    // try {
    mActivityManager->UpdateConfiguration(NULL);
    // } catch (RemoteException e) {
    // }
    return NOERROR;
}

void CWindowManagerService::DisplayReady(
    /* [in] */ Int32 displayId)
{
    synchronized (mWindowMapLock) {
        AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
        if (displayContent != NULL) {
            mAnimator->AddDisplayLocked(displayId);
            {
                AutoLock lock(displayContent->mDisplaySizeLock);
                // Bootstrap the default logical display from the display manager.
                AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
                AutoPtr<IDisplayInfo> newDisplayInfo;
                mDisplayManagerInternal->GetDisplayInfo(displayId, (IDisplayInfo**)&newDisplayInfo);
                if (newDisplayInfo != NULL) {
                    displayInfo->CopyFrom(newDisplayInfo);
                }
                displayInfo->GetLogicalWidth(&displayContent->mInitialDisplayWidth);
                displayInfo->GetLogicalHeight(&displayContent->mInitialDisplayHeight);
                displayInfo->GetLogicalDensityDpi(&displayContent->mInitialDisplayDensity);
                displayContent->mBaseDisplayWidth = displayContent->mInitialDisplayWidth;
                displayContent->mBaseDisplayHeight = displayContent->mInitialDisplayHeight;
                displayContent->mBaseDisplayDensity = displayContent->mInitialDisplayDensity;
                displayContent->mBaseDisplayRect->Set(0, 0,
                        displayContent->mBaseDisplayWidth, displayContent->mBaseDisplayHeight);
            }
        }
    }
}

ECode CWindowManagerService::SystemReady()
{
    return mPolicy->SystemReady();
}

// -------------------------------------------------------------
// Async Handler
// -------------------------------------------------------------
const Int32 CWindowManagerService::H::REPORT_FOCUS_CHANGE = 2;
const Int32 CWindowManagerService::H::REPORT_LOSING_FOCUS = 3;
const Int32 CWindowManagerService::H::DO_TRAVERSAL = 4;
const Int32 CWindowManagerService::H::ADD_STARTING = 5;
const Int32 CWindowManagerService::H::REMOVE_STARTING = 6;
const Int32 CWindowManagerService::H::FINISHED_STARTING = 7;
const Int32 CWindowManagerService::H::REPORT_APPLICATION_TOKEN_WINDOWS = 8;
const Int32 CWindowManagerService::H::REPORT_APPLICATION_TOKEN_DRAWN = 9;
const Int32 CWindowManagerService::H::WINDOW_FREEZE_TIMEOUT = 11;
const Int32 CWindowManagerService::H::APP_TRANSITION_TIMEOUT = 13;
const Int32 CWindowManagerService::H::PERSIST_ANIMATION_SCALE = 14;
const Int32 CWindowManagerService::H::FORCE_GC = 15;
const Int32 CWindowManagerService::H::ENABLE_SCREEN = 16;
const Int32 CWindowManagerService::H::APP_FREEZE_TIMEOUT = 17;
const Int32 CWindowManagerService::H::SEND_NEW_CONFIGURATION = 18;
const Int32 CWindowManagerService::H::REPORT_WINDOWS_CHANGE = 19;
const Int32 CWindowManagerService::H::DRAG_START_TIMEOUT = 20;
const Int32 CWindowManagerService::H::DRAG_END_TIMEOUT = 21;
const Int32 CWindowManagerService::H::REPORT_HARD_KEYBOARD_STATUS_CHANGE = 22;
const Int32 CWindowManagerService::H::BOOT_TIMEOUT = 23;
const Int32 CWindowManagerService::H::WAITING_FOR_DRAWN_TIMEOUT = 24;
const Int32 CWindowManagerService::H::SHOW_STRICT_MODE_VIOLATION = 25;
const Int32 CWindowManagerService::H::DO_ANIMATION_CALLBACK = 26;
const Int32 CWindowManagerService::H::DO_DISPLAY_ADDED = 27;
const Int32 CWindowManagerService::H::DO_DISPLAY_REMOVED = 28;
const Int32 CWindowManagerService::H::DO_DISPLAY_CHANGED = 29;
const Int32 CWindowManagerService::H::CLIENT_FREEZE_TIMEOUT = 30;
const Int32 CWindowManagerService::H::TAP_OUTSIDE_STACK = 31;
const Int32 CWindowManagerService::H::NOTIFY_ACTIVITY_DRAWN = 32;
const Int32 CWindowManagerService::H::ALL_WINDOWS_DRAWN = 33;
const Int32 CWindowManagerService::H::NEW_ANIMATOR_SCALE = 34;
const Int32 CWindowManagerService::H::SHOW_CIRCULAR_DISPLAY_MASK = 35;
const Int32 CWindowManagerService::H::SHOW_EMULATOR_DISPLAY_OVERLAY = 36;
const Int32 CWindowManagerService::H::CHECK_IF_BOOT_ANIMATION_FINISHED = 37;

ECode CWindowManagerService::H::HandleMessage(
    /* [in] */ IMessage* msg)
{
    Int32 what, arg1, arg2;
    msg->GetWhat(&what);
    msg->GetArg1(&arg1);
    msg->GetArg2(&arg2);

    if (CWindowManagerService::DEBUG_WINDOW_TRACE) {
        Slogger::V(CWindowManagerService::TAG, "handleMessage: entry what=%d", what);
    }

    switch (what) {
        case REPORT_FOCUS_CHANGE: {
            mHost->HandleReportFocusChange();
            break;
        }
        case REPORT_LOSING_FOCUS: {
            mHost->HandleReportLosingFocus();
            break;
        }
        case DO_TRAVERSAL: {
            mHost->HandleDoTraversal();
            break;
        }
        case ADD_STARTING: {
            AutoPtr<IInterface> obj;
            msg->GetObj((IInterface**)&obj);
            AppWindowToken* wtoken = (AppWindowToken*)(IObject*)obj.Get();
            mHost->HandleAddStarting(wtoken);
            break;
        }
        case REMOVE_STARTING: {
            AutoPtr<IInterface> obj;
            msg->GetObj((IInterface**)&obj);
            AppWindowToken* wtoken = (AppWindowToken*)(IObject*)obj.Get();
            mHost->HandleRemoveStarting(wtoken);
            break;
        }
        case FINISHED_STARTING: {
            mHost->HandleFinishedStarting();
            break;
        }
        case REPORT_APPLICATION_TOKEN_DRAWN: {
            AutoPtr<IInterface> obj;
            msg->GetObj((IInterface**)&obj);
            AppWindowToken* wtoken = (AppWindowToken*)(IObject*)obj.Get();
            mHost->HandleReportApplicationTokenDrawn(wtoken);
            break;
        }
        case REPORT_APPLICATION_TOKEN_WINDOWS: {
            AutoPtr<IInterface> obj;
            msg->GetObj((IInterface**)&obj);
            AppWindowToken* wtoken = (AppWindowToken*)(IObject*)obj.Get();
            Boolean nowVisible = arg1 != 0;
            Boolean nowGone = arg2 != 0;
            mHost->HandleReportApplicationTokenWindows(nowVisible, nowGone, wtoken);
            break;
        }
        case WINDOW_FREEZE_TIMEOUT: {
            mHost->HandleWindowFreezeTimeout();
            break;
        }
        case APP_TRANSITION_TIMEOUT: {
            mHost->HandleAppTransitionTimeout();
            break;
        }
        case PERSIST_ANIMATION_SCALE: {
            mHost->HandlePersistAnimationScale();
            break;
        }
        case FORCE_GC: {
            mHost->HandleForceGc();
            break;
        }
        case ENABLE_SCREEN: {
            mHost->PerformEnableScreen();
            break;
        }
        case APP_FREEZE_TIMEOUT: {
            mHost->HandleAppFreezeTimeout();
            break;
        }
        case CLIENT_FREEZE_TIMEOUT: {
            mHost->HandleClientFreezeTimeout();
            break;
        }
        case SEND_NEW_CONFIGURATION: {
            RemoveMessages(SEND_NEW_CONFIGURATION);
            mHost->SendNewConfiguration();
            break;
        }
        case REPORT_WINDOWS_CHANGE: {
            mHost->HandleReportWindowsChange();
            break;
        }
        case DRAG_START_TIMEOUT: {
            AutoPtr<IInterface> obj;
            msg->GetObj((IInterface**)&obj);
            AutoPtr<IBinder> binder = IBinder::Probe(obj);
            mHost->HandleDragStartTimeout(binder);
            break;
        }
        case DRAG_END_TIMEOUT: {
            AutoPtr<IInterface> obj;
            msg->GetObj((IInterface**)&obj);
            AutoPtr<IBinder> binder = IBinder::Probe(obj);
            mHost->HandleDragEndTimeout(binder);
            break;
        }
        case REPORT_HARD_KEYBOARD_STATUS_CHANGE: {
            mHost->NotifyHardKeyboardStatusChange();
            break;
        }
        case BOOT_TIMEOUT: {
            mHost->PerformBootTimeout();
            break;
        }
        case WAITING_FOR_DRAWN_TIMEOUT: {
            mHost->HandleWaitingForDrawnTimeout();
            break;
        }
        case SHOW_STRICT_MODE_VIOLATION: {
            mHost->ShowStrictModeViolation(arg1, arg2);
            break;
        }
        case SHOW_CIRCULAR_DISPLAY_MASK: {
            mHost->ShowCircularMask();
            break;
        }
        case SHOW_EMULATOR_DISPLAY_OVERLAY: {
            mHost->ShowEmulatorDisplayOverlay();
            break;
        }
        case DO_ANIMATION_CALLBACK: {
            AutoPtr<IInterface> obj;
            msg->GetObj((IInterface**)&obj);
            AutoPtr<IIRemoteCallback> callback = IIRemoteCallback::Probe(obj);
            callback->SendResult(NULL);
            break;
        }
        case DO_DISPLAY_ADDED: {
            mHost->HandleDisplayAdded(arg1);
            break;
        }
        case DO_DISPLAY_REMOVED: {
            {
                AutoLock lock(mHost->mWindowMapLock);
                mHost->HandleDisplayRemovedLocked(arg1);
            }
            break;
        }
        case DO_DISPLAY_CHANGED: {
            {
                AutoLock lock(mHost->mWindowMapLock);
                mHost->HandleDisplayChangedLocked(arg1);
            }
            break;
        }
        case TAP_OUTSIDE_STACK: {
            Int32 stackId;
            {
                AutoLock lock(mHost->mWindowMapLock);
                AutoPtr<IInterface> obj;
                msg->GetObj((IInterface**)&obj);
                AutoPtr<DisplayContent> dc = (DisplayContent*)(IObject*)obj.Get();
                stackId = dc->StackIdFromPoint(arg1, arg2);
            }
            if (stackId >= 0) {
                // try {
                mHost->mActivityManager->SetFocusedStack(stackId);
                // } catch (RemoteException e) {
                // }
            }
            break;
        }
        case NOTIFY_ACTIVITY_DRAWN: {
            // try {
            AutoPtr<IInterface> obj;
            msg->GetObj((IInterface**)&obj);
            AutoPtr<IBinder> binder = IBinder::Probe(obj);
            mHost->mActivityManager->NotifyActivityDrawn(binder);
            // } catch (RemoteException e) {
            // }
            break;
        }
        case ALL_WINDOWS_DRAWN: {
            AutoPtr<IRunnable> callback;
            {
                AutoLock lock(mHost->mWindowMapLock);
                callback = mHost->mWaitingForDrawnCallback;
                mHost->mWaitingForDrawnCallback = NULL;
            }
            if (callback != NULL) {
                callback->Run();
            }
        }
        case NEW_ANIMATOR_SCALE: {
            Float scale;
            mHost->GetCurrentAnimatorScale(&scale);
            AutoPtr<IValueAnimatorHelper> helper;
            CValueAnimatorHelper::AcquireSingleton((IValueAnimatorHelper**)&helper);
            helper->SetDurationScale(scale);
            AutoPtr<IInterface> obj;
            msg->GetObj((IInterface**)&obj);

            AutoPtr<IWindowSession> ws = IWindowSession::Probe(obj);
            AutoPtr<CSession> session = (CSession*)ws.Get();
            if (session != NULL) {
                // try {
                session->mCallback->OnAnimatorScaleChanged(scale);
                // } catch (RemoteException e) {
                // }
            }
            else {
                List<AutoPtr<IIWindowSessionCallback> > callbacks;
                {
                    AutoLock lock(mHost->mWindowMapLock);
                    Int32 size;
                    ISet::Probe(mHost->mSessions)->GetSize(&size);
                    for (Int32 i = 0; i < size; i++) {
                        AutoPtr<IInterface> value;
                        mHost->mSessions->GetValueAt(i, (IInterface**)&value);
                        AutoPtr<CSession> s = (CSession*)(IWindowSession*)value.Get();
                        callbacks.PushBack(s->mCallback);
                    }

                }
                List<AutoPtr<IIWindowSessionCallback> >::Iterator it = callbacks.Begin();
                for (; it != callbacks.End(); ++it) {
                    // try {
                    (*it)->OnAnimatorScaleChanged(scale);
                    // } catch (RemoteException e) {
                    // }
                }
            }
            break;
        }
        case CHECK_IF_BOOT_ANIMATION_FINISHED: {
            Boolean bootAnimationComplete;
            {
                AutoLock lock(mHost->mWindowMapLock);
                if (DEBUG_BOOT) Slogger::I(TAG, "CHECK_IF_BOOT_ANIMATION_FINISHED:");
                bootAnimationComplete = mHost->CheckBootAnimationCompleteLocked();
            }
            if (bootAnimationComplete) {
                mHost->PerformEnableScreen();
            }
        }
        break;
    }

    if (CWindowManagerService::DEBUG_WINDOW_TRACE) {
        Slogger::V(CWindowManagerService::TAG, "handleMessage: exit");
    }
    return NOERROR;
}

ECode CWindowManagerService::HandleReportFocusChange()
{
    AutoPtr<WindowState> lastFocus;
    AutoPtr<WindowState> newFocus;

    synchronized (mWindowMapLock) {
        lastFocus = mLastFocus;
        newFocus = mCurrentFocus;
        if (lastFocus == newFocus) {
            // Focus is not changing, so nothing to do.
            return NOERROR;
        }

        mLastFocus = newFocus;
        if (DEBUG_FOCUS_LIGHT)
            Slogger::I(TAG, "Focus moving from %p to %p", lastFocus.Get(), newFocus.Get());

        Boolean isDisplayed;
        if (newFocus != NULL && lastFocus != NULL
                && (newFocus->IsDisplayedLw(&isDisplayed), !isDisplayed)) {
            Slogger::I(TAG, "Delaying loss of focus...");
            mLosingFocus.PushBack(lastFocus);
            lastFocus = NULL;
        }
    }

    //System.out.println("Changing focus from " + lastFocus
    //                   + " to " + newFocus);
    if (newFocus != NULL) {
        if (DEBUG_FOCUS_LIGHT) Slogger::I(TAG, "Gaining focus: %p", newFocus.Get());
        newFocus->ReportFocusChangedSerialized(TRUE, mInTouchMode);
        NotifyFocusChanged();
    }

    if (lastFocus != NULL) {
        if (DEBUG_FOCUS_LIGHT) Slogger::I(TAG, "Losing focus: %p", lastFocus.Get());
        lastFocus->ReportFocusChangedSerialized(FALSE, mInTouchMode);
    }

    return NOERROR;
}

ECode CWindowManagerService::HandleReportLosingFocus()
{
    WindowList losers;

    synchronized (mWindowMapLock) {
        losers.Assign(mLosingFocus.Begin(), mLosingFocus.End());
        mLosingFocus.Clear();
    }

    WindowList::Iterator it;
    for (it = losers.Begin(); it != losers.End(); ++it) {
        if (DEBUG_FOCUS_LIGHT) Slogger::I(TAG, "Losing delayed focus: %p", (*it).Get());
        (*it)->ReportFocusChangedSerialized(FALSE, mInTouchMode);
    }

    return NOERROR;
}

ECode CWindowManagerService::HandleDoTraversal()
{
    AutoLock lock(mWindowMapLock);
    mTraversalScheduled = FALSE;
    PerformLayoutAndPlaceSurfacesLocked();
    return NOERROR;
}

ECode CWindowManagerService::HandleAddStarting(
    /* [in] */ AppWindowToken* wtoken)
{
    AutoPtr<StartingData> sd = wtoken->mStartingData;

    if (sd == NULL) {
        // Animation has been canceled... do nothing.
        return NOERROR;
    }

    if (DEBUG_STARTING_WINDOW) {
        Slogger::V(TAG, "Add starting %p: pkg=%s", wtoken, sd->mPkg.string());
    }

    AutoPtr<IView> view;
    if (FAILED(mPolicy->AddStartingWindow(
            wtoken->mToken, sd->mPkg,
            sd->mTheme, sd->mCompatInfo, sd->mNonLocalizedLabel, sd->mLabelRes,
            sd->mIcon, sd->mLogo, sd->mWindowFlags, (IView**)&view))) {
        Slogger::W(TAG, "Exception when adding starting window");
    }

    if (view != NULL) {
        Boolean abort = FALSE;

        synchronized (mWindowMapLock) {

            if (wtoken->mRemoved || wtoken->mStartingData == NULL) {
                // If the window was successfully added, then
                // we need to remove it.
                if (wtoken->mStartingWindow != NULL) {
                    if (DEBUG_STARTING_WINDOW) {
                        Slogger::V(TAG, "Aborted starting %p: removed=%d  startingData=%p",
                             wtoken, wtoken->mRemoved, wtoken->mStartingData.Get());
                    }
                    wtoken->mStartingWindow = NULL;
                    wtoken->mStartingData = NULL;
                    abort = TRUE;
                }
            }
            else {
                wtoken->mStartingView = view;
            }

            if (DEBUG_STARTING_WINDOW) {
                Slogger::V(TAG, "Added starting %p: startingWindow=%p, startingView=%p",
                    wtoken, wtoken->mStartingWindow.Get(), wtoken->mStartingView.Get());
            }
        }

        if (abort) {
            if (FAILED(mPolicy->RemoveStartingWindow(wtoken->mToken, view))) {
                Slogger::W(TAG, "Exception when removing starting window");
            }
        }
    }

    return NOERROR;
}

ECode CWindowManagerService::HandleRemoveStarting(
    /* [in] */ AppWindowToken* wtoken)
{
    AutoPtr<IBinder> token;
    AutoPtr<IView> view;

    synchronized (mWindowMapLock) {

        if (DEBUG_STARTING_WINDOW) {
            Slogger::V(TAG, "Remove starting %p: startingWindow=%p, startingView=%p",
                wtoken, wtoken->mStartingWindow.Get(), wtoken->mStartingView.Get());
        }

        if (wtoken->mStartingWindow != NULL) {
            view = wtoken->mStartingView;
            token = wtoken->mToken;
            wtoken->mStartingData = NULL;
            wtoken->mStartingView = NULL;
            wtoken->mStartingWindow = NULL;
            wtoken->mStartingDisplayed = FALSE;
        }
    }

    if (view != NULL) {
        if (FAILED(mPolicy->RemoveStartingWindow(token, view))) {
            Slogger::W(TAG, "Exception when removing starting window");
        }
    }

    return NOERROR;
}

ECode CWindowManagerService::HandleFinishedStarting()
{
    AutoPtr<IBinder> token;
    AutoPtr<IView> view;
    while (TRUE) {
        synchronized (mWindowMapLock) {
            if (mFinishedStarting.Begin() == mFinishedStarting.End()) {
                break;
            }

            AutoPtr<AppWindowToken> wtoken = mFinishedStarting.GetBack();
            mFinishedStarting.PopBack();

            if (DEBUG_STARTING_WINDOW) {
                Slogger::V(TAG, "Finished starting %p: startingWindow=%p startingView=%p",
                    wtoken.Get(), wtoken->mStartingWindow.Get(), wtoken->mStartingView.Get());
            }

            if (wtoken->mStartingWindow == NULL) {
                continue;
            }

            view = wtoken->mStartingView;
            token = wtoken->mToken;
            wtoken->mStartingData = NULL;
            wtoken->mStartingView = NULL;
            wtoken->mStartingWindow = NULL;
            wtoken->mStartingDisplayed = FALSE;
        }

        if (FAILED(mPolicy->RemoveStartingWindow(token, view))) {
            Slogger::W(TAG, "Exception when removing starting window");
        }
    }

    return NOERROR;
}

ECode CWindowManagerService::HandleReportApplicationTokenDrawn(
    /* [in] */ AppWindowToken* wtoken)
{
    // try {
    // if (DEBUG_VISIBILITY) Slog.v(
    //         TAG, "Reporting drawn in " + wtoken);
    ECode ec = wtoken->mAppToken->WindowsDrawn();
    // } catch (RemoteException ex) {
    // }
    return ec;
}

ECode CWindowManagerService::HandleReportApplicationTokenWindows(
    /* [in] */ Boolean nowVisible,
    /* [in] */ Boolean nowGone,
    /* [in] */ AppWindowToken* wtoken)
{
    // try {
    // if (DEBUG_VISIBILITY) Slog.v(
    //         TAG, "Reporting visible in " + wtoken
    //         + " visible=" + nowVisible
    //         + " gone=" + nowGone);
    ECode ec = NOERROR;
    if (nowVisible) {
        ec = wtoken->mAppToken->WindowsVisible();
    }
    else {
        ec = wtoken->mAppToken->WindowsGone();
    }
    // } catch (RemoteException ex) {
    // }
    return ec;
}

ECode CWindowManagerService::HandleWindowFreezeTimeout()
{
    // TODO(multidisplay): Can non-default displays rotate?
    synchronized (mWindowMapLock) {
        Slogger::W(TAG, "Window freeze timeout expired.");

        AutoPtr<WindowList> windows = GetDefaultWindowListLocked();
        WindowList::ReverseIterator rit = windows->RBegin();
        for (; rit != windows->REnd(); ++rit) {
            AutoPtr<WindowState> w = *rit;
            if (w->mOrientationChanging) {
                w->mOrientationChanging = FALSE;
                w->mLastFreezeDuration = (Int32)(SystemClock::GetElapsedRealtime() - mDisplayFreezeTime);
                Slogger::W(TAG, "Force clearing orientation change: %p", w.Get());
            }
        }

        PerformLayoutAndPlaceSurfacesLocked();
    }

    return NOERROR;
}

ECode CWindowManagerService::HandleAppTransitionTimeout()
{
    synchronized (mWindowMapLock) {
        if (mAppTransition->IsTransitionSet()) {
            if (DEBUG_APP_TRANSITIONS) Slogger::V(TAG, "*** APP TRANSITION TIMEOUT");
            mAppTransition->SetTimeout();
            PerformLayoutAndPlaceSurfacesLocked();
        }
    }

    return NOERROR;
}

ECode CWindowManagerService::HandlePersistAnimationScale()
{
    AutoPtr<IContentResolver> cr;
    mContext->GetContentResolver((IContentResolver**)&cr);
    Boolean result;
    Settings::Global::PutFloat(cr, ISettingsGlobal::WINDOW_ANIMATION_SCALE, mWindowAnimationScaleSetting, &result);
    Settings::Global::PutFloat(cr, ISettingsGlobal::TRANSITION_ANIMATION_SCALE, mTransitionAnimationScaleSetting, &result);
    Settings::Global::PutFloat(cr, ISettingsGlobal::ANIMATOR_DURATION_SCALE, mAnimatorDurationScaleSetting, &result);

    return NOERROR;
}

ECode CWindowManagerService::HandleForceGc()
{
    synchronized (mWindowMapLock) {
        // Since we're holding both mWindowMap and mAnimator we don't need to
        // hold mAnimator.mLayoutToAnim.
        if (mAnimator->mAnimating || mAnimationScheduled) {
            // If we are animating, don't do the gc now but
            // delay a bit so we don't interrupt the animation.
            Boolean result;
            mH->SendEmptyMessageDelayed(H::FORCE_GC, 2000, &result);
            return NOERROR;
        }
        // If we are currently rotating the display, it will
        // schedule a new message when done.
        if (mDisplayFrozen) {
            return NOERROR;
        }
    }

    //Runtime.getRuntime().gc();

    return NOERROR;
}

ECode CWindowManagerService::HandleAppFreezeTimeout()
{
    synchronized (mWindowMapLock) {
        Slogger::W(TAG, "App freeze timeout expired.");
        Int32 numStacks;
        mStackIdToStack->GetSize(&numStacks);
        for (Int32 stackNdx = 0; stackNdx < numStacks; ++stackNdx) {
            AutoPtr<IInterface> value;
            mStackIdToStack->ValueAt(stackNdx, (IInterface**)&value);
            AutoPtr<TaskStack> stack = (TaskStack*)(IObject*)value.Get();
            AutoPtr<List<AutoPtr<Task> > > tasks = stack->GetTasks();
            List<AutoPtr<Task> >::ReverseIterator rit = tasks->RBegin();
            for (; rit != tasks->REnd(); ++rit) {
                AppTokenList tokens = (*rit)->mAppTokens;
                AppTokenList::ReverseIterator rit = tokens.RBegin();
                for (; rit != tokens.REnd(); ++rit) {
                    AutoPtr<AppWindowToken> tok = *rit;
                    if (tok->mAppAnimator->mFreezingScreen) {
                        Slogger::W(TAG, "Force clearing freeze: %p", tok.Get());
                        UnsetAppFreezingScreenLocked(tok, TRUE, TRUE);
                    }
                }
            }
        }
    }
    return NOERROR;
}

ECode CWindowManagerService::HandleClientFreezeTimeout()
{
    synchronized (mWindowMapLock) {
        if (mClientFreezingScreen) {
            mClientFreezingScreen = FALSE;
            AutoPtr<ICharSequence> cs;
            CString::New(String("client-timeout"), (ICharSequence**)&cs);
            mLastFinishedFreezeSource = IObject::Probe(cs);
            StopFreezingDisplayLocked();
        }
    }

    return NOERROR;
}

ECode CWindowManagerService::HandleReportWindowsChange()
{
    if (mWindowsChanged) {
        synchronized (mWindowMapLock) {
            mWindowsChanged = FALSE;
        }
        NotifyWindowsChanged();
    }

    return NOERROR;
}

ECode CWindowManagerService::HandleDragStartTimeout(
    /* [in] */ IBinder* winBinder)
{
    if (DEBUG_DRAG) {
        Slogger::W(TAG, "Timeout starting drag by win %p", winBinder);
    }
    synchronized (mWindowMapLock) {
        // !!! TODO: ANR the app that has failed to start the drag in time
        if (mDragState != NULL) {
            mDragState->Unregister();
            mInputMonitor->UpdateInputWindowsLw(TRUE /*force*/);
            mDragState->Reset();
            mDragState = NULL;
        }
    }
    return NOERROR;
}

ECode CWindowManagerService::HandleDragEndTimeout(
    /* [in] */ IBinder* winBinder)
{
    if (DEBUG_DRAG) {
        Slogger::W(TAG, "Timeout ending drag to win %p", winBinder);
    }
    synchronized (mWindowMapLock) {
        // !!! TODO: ANR the drag-receiving app
        if (mDragState != NULL) {
            mDragState->mDragResult = FALSE;
            mDragState->EndDragLw();
        }
    }

    return NOERROR;
}

ECode CWindowManagerService::HandleWaitingForDrawnTimeout()
{
    AutoPtr<IRunnable> callback;
    synchronized (mWindowMapLock) {
        // Slogger::W(TAG, "Timeout waiting for drawn: undrawn=" + mWaitingForDrawn);
        mWaitingForDrawn.Clear();
        callback = mWaitingForDrawnCallback;
        mWaitingForDrawnCallback = NULL;
    }
    if (callback != NULL) {
        callback->Run();
    }
    return NOERROR;
}

// -------------------------------------------------------------
// IWindowManager API
// -------------------------------------------------------------
ECode CWindowManagerService::OpenSession(
    /* [in] */ IIWindowSessionCallback* callback,
    /* [in] */ IInputMethodClient* client,
    /* [in] */ IIInputContext* inputContext,
    /* [out] */ IWindowSession** session)
{
    VALIDATE_NOT_NULL(session)
    *session = NULL;

    if (client == NULL || inputContext == NULL) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
        // throw new IllegalArgumentException("NULL client");
        // throw new IllegalArgumentException("NULL inputContext");
    }

    return CSession::New((Handle64)this, callback, client, inputContext, session);
}

ECode CWindowManagerService::InputMethodClientHasFocus(
    /* [in] */ IInputMethodClient* client,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    synchronized (mWindowMapLock) {
        // The focus for the client is the window immediately below
        // where we would place the input method window.
        AutoPtr<WindowList> windows = GetDefaultWindowListLocked();
        WindowList::Iterator it = FindDesiredInputMethodWindowItLocked(FALSE);
        if (it != windows->End() && it != windows->Begin()) {
            // TODO(multidisplay): IMEs are only supported on the default display.
            AutoPtr<WindowState> imFocus = *(--it);
            // if (DEBUG_INPUT_METHOD) {
            //     Slog.i(TAG, "Desired input method target: " + imFocus);
            //     Slog.i(TAG, "Current focus: " + mCurrentFocus);
            //     Slog.i(TAG, "Last focus: " + mLastFocus);
            // }
            if (imFocus != NULL) {
                // This may be a starting window, in which case we still want
                // to count it as okay.
                Int32 type;
                imFocus->mAttrs->GetType(&type);
                if (type == IWindowManagerLayoutParams::TYPE_APPLICATION_STARTING
                        && imFocus->mAppToken != NULL) {
                    // The client has definitely started, so it really should
                    // have a window in this app token.  Let's look for it.
                    WindowList::Iterator wit
                            = imFocus->mAppToken->mWindows.Begin();
                    for (; wit != imFocus->mAppToken->mWindows.End(); ++wit) {
                        AutoPtr<WindowState> w = *wit;
                        if (w != imFocus) {
                            Slogger::I(TAG, "Switching to real app window: %d", w.Get());
                            imFocus = w;
                            break;
                        }
                    }
                }
                // if (DEBUG_INPUT_METHOD) {
                //     Slog.i(TAG, "IM target client: " + imFocus.mSession.mClient);
                //     if (imFocus.mSession.mClient != null) {
                //         Slog.i(TAG, "IM target client binder: "
                //                 + imFocus.mSession.mClient.asBinder());
                //         Slog.i(TAG, "Requesting client binder: " + client.asBinder());
                //     }
                // }
                if (imFocus->mSession->mClient != NULL &&
                        imFocus->mSession->mClient.Get() == client) {
                    *result = TRUE;
                    return NOERROR;
                }
            }
        }

        // Okay, how about this...  what is the current focus?
        // It seems in some cases we may not have moved the IM
        // target window, such as when it was in a pop-up window,
        // so let's also look at the current focus.  (An example:
        // go to Gmail, start searching so the keyboard goes up,
        // press home.  Sometimes the IME won't go down.)
        // Would be nice to fix this more correctly, but it's
        // way at the end of a release, and this should be good enough.
        if (mCurrentFocus != NULL && mCurrentFocus->mSession->mClient != NULL
                && mCurrentFocus->mSession->mClient.Get() == client) {
            *result = TRUE;
            return NOERROR;
        }
    }
    *result = FALSE;
    return NOERROR;
}

ECode CWindowManagerService::GetInitialDisplaySize(
    /* [in] */ Int32 displayId,
    /* [in] */ IPoint* inSize,
    /* [out] */ IPoint** outSize)
{
    VALIDATE_NOT_NULL(outSize)
    synchronized (mWindowMapLock) {
        AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
        if (displayContent != NULL && displayContent->HasAccess(Binder::GetCallingUid())) {
            AutoLock lock(displayContent->mDisplaySizeLock);
            inSize->Set(displayContent->mInitialDisplayWidth, displayContent->mInitialDisplayHeight);
        }
    }
    *outSize = inSize;
    REFCOUNT_ADD(*outSize)
    return NOERROR;
}

ECode CWindowManagerService::GetBaseDisplaySize(
    /* [in] */ Int32 displayId,
    /* [in] */ IPoint* inSize,
    /* [out] */ IPoint** outSize)
{
    VALIDATE_NOT_NULL(outSize)
    synchronized (mWindowMapLock) {
        AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
        if (displayContent != NULL && displayContent->HasAccess(Binder::GetCallingUid())) {
            AutoLock lock(displayContent->mDisplaySizeLock);
            inSize->Set(displayContent->mBaseDisplayWidth, displayContent->mBaseDisplayHeight);
        }
    }
    *outSize = inSize;
    REFCOUNT_ADD(*outSize)
    return NOERROR;
}

ECode CWindowManagerService::SetForcedDisplaySize(
    /* [in] */ Int32 displayId,
    /* [in] */ Int32 width,
    /* [in] */ Int32 height)
{
    Int32 perm;
    FAIL_RETURN(mContext->CheckCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::WRITE_SECURE_SETTINGS, &perm));
    if (perm != IPackageManager::PERMISSION_GRANTED) {
        Slogger::E(TAG, "Must hold permission %s", Elastos::Droid::Manifest::permission::WRITE_SECURE_SETTINGS.string());
        return E_SECURITY_EXCEPTION;
    }
    if (displayId != IDisplay::DEFAULT_DISPLAY) {
        Slogger::E(TAG, "Can only set the default display");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    Int64 ident = Binder::ClearCallingIdentity();
    // try {
    synchronized (mWindowMapLock) {
        // Set some sort of reasonable bounds on the size of the display that we
        // will try to emulate.
        Int32 MIN_WIDTH = 200;
        Int32 MIN_HEIGHT = 200;
        Int32 MAX_SCALE = 2;
        AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
        if (displayContent != NULL) {
            width = Elastos::Core::Math::Min(Elastos::Core::Math::Max(width, MIN_WIDTH),
                    displayContent->mInitialDisplayWidth * MAX_SCALE);
            height = Elastos::Core::Math::Min(Elastos::Core::Math::Max(height, MIN_HEIGHT),
                    displayContent->mInitialDisplayHeight * MAX_SCALE);
            SetForcedDisplaySizeLocked(displayContent, width, height);
            AutoPtr<IContentResolver> cr;
            mContext->GetContentResolver((IContentResolver**)&cr);
            Boolean result;
            Settings::Global::PutString(cr, ISettingsGlobal::DISPLAY_SIZE_FORCED,
                    StringUtils::ToString(width) + "," + StringUtils::ToString(height), &result);
        }
    }
    // } finally {
    //     Binder.restoreCallingIdentity(ident);
    // }
    Binder::RestoreCallingIdentity(ident);
    return NOERROR;
}

void CWindowManagerService::ReadForcedDisplaySizeAndDensityLocked(
    /* [in] */ DisplayContent* displayContent)
{
    AutoPtr<IContentResolver> cr;
    mContext->GetContentResolver((IContentResolver**)&cr);
    String sizeStr;
    Settings::Global::GetString(cr, ISettingsGlobal::DISPLAY_SIZE_FORCED, &sizeStr);
    if (sizeStr.IsNullOrEmpty()) {
        AutoPtr<ISystemProperties> sysProp;
        CSystemProperties::AcquireSingleton((ISystemProperties**)&sysProp);
        sysProp->Get(SIZE_OVERRIDE, String(NULL), &sizeStr);
    }
    if (!sizeStr.IsNullOrEmpty()) {
        Int32 pos = sizeStr.IndexOf(',');
        if (pos > 0 && sizeStr.LastIndexOf(',') == pos) {
            Int32 width, height;
            // try {
            width = StringUtils::ParseInt32(sizeStr.Substring(0, pos));
            height = StringUtils::ParseInt32(sizeStr.Substring(pos+1));
            AutoLock lock(displayContent->mDisplaySizeLock);
            if (displayContent->mBaseDisplayWidth != width
                    || displayContent->mBaseDisplayHeight != height) {
                // Slog.i(TAG, "FORCED DISPLAY SIZE: " + width + "x" + height);
                displayContent->mBaseDisplayWidth = width;
                displayContent->mBaseDisplayHeight = height;
            }
            // } catch (NumberFormatException ex) {
            // }
        }
    }


    String densityStr;
    Settings::Global::GetString(cr, ISettingsGlobal::DISPLAY_DENSITY_FORCED, &densityStr);
    if (densityStr.IsNullOrEmpty()) {
        AutoPtr<ISystemProperties> sysProp;
        CSystemProperties::AcquireSingleton((ISystemProperties**)&sysProp);
        sysProp->Get(SIZE_OVERRIDE, String(NULL), &densityStr);
    }
    if (!densityStr.IsNullOrEmpty()) {
        Int32 density;
        // try {
        density = StringUtils::ParseInt32(densityStr);
        AutoLock lock(displayContent->mDisplaySizeLock);
        if (displayContent->mBaseDisplayDensity != density) {
            Slogger::I(TAG, "FORCED DISPLAY DENSITY: %d", density);
            displayContent->mBaseDisplayDensity = density;
        }
        // } catch (NumberFormatException ex) {
        // }
    }
}

void CWindowManagerService::SetForcedDisplaySizeLocked(
    /* [in] */ DisplayContent* displayContent,
    /* [in] */ Int32 width,
    /* [in] */ Int32 height)
{
    Slogger::I(TAG, "Using new display size: %dx%d", width, height);
    {
        AutoLock lock(displayContent->mDisplaySizeLock);
        displayContent->mBaseDisplayWidth = width;
        displayContent->mBaseDisplayHeight = height;
    }
    ReconfigureDisplayLocked(displayContent);
}

ECode CWindowManagerService::ClearForcedDisplaySize(
    /* [in] */ Int32 displayId)
{
    Int32 perm;
    FAIL_RETURN(mContext->CheckCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::WRITE_SECURE_SETTINGS, &perm));
    if (perm != IPackageManager::PERMISSION_GRANTED) {
        Slogger::E(TAG, "Must hold permission %s", Elastos::Droid::Manifest::permission::WRITE_SECURE_SETTINGS.string());
        return E_SECURITY_EXCEPTION;
    }

    if (displayId != IDisplay::DEFAULT_DISPLAY) {
        Slogger::E(TAG, "Can only set the default display");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    Int64 ident = Binder::ClearCallingIdentity();
    // try {
    synchronized (mWindowMapLock) {
        AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
        if (displayContent != NULL) {
            SetForcedDisplaySizeLocked(displayContent, displayContent->mInitialDisplayWidth,
                    displayContent->mInitialDisplayHeight);
            AutoPtr<IContentResolver> cr;
            mContext->GetContentResolver((IContentResolver**)&cr);
            Boolean result;
            Settings::Global::PutString(cr, ISettingsGlobal::DISPLAY_SIZE_FORCED, String(""), &result);
        }
    }
    // } finally {
    //     Binder.restoreCallingIdentity(ident);
    // }
    Binder::RestoreCallingIdentity(ident);
    return NOERROR;
}

ECode CWindowManagerService::GetInitialDisplayDensity(
    /* [in] */ Int32 displayId,
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result)
    *result = -1;

    synchronized (mWindowMapLock) {
        AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
        if (displayContent != NULL && displayContent->HasAccess(Binder::GetCallingUid())) {
            AutoLock lock(displayContent->mDisplaySizeLock);
            *result = displayContent->mInitialDisplayDensity;
        }
    }
    return NOERROR;
}

ECode CWindowManagerService::GetBaseDisplayDensity(
    /* [in] */ Int32 displayId,
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result)
    *result = -1;

    synchronized (mWindowMapLock) {
        AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
        if (displayContent != NULL && displayContent->HasAccess(Binder::GetCallingUid())) {
            AutoLock lock(displayContent->mDisplaySizeLock);
            *result = displayContent->mBaseDisplayDensity;
        }
    }
    return NOERROR;
}

ECode CWindowManagerService::SetForcedDisplayDensity(
    /* [in] */ Int32 displayId,
    /* [in] */ Int32 density)
{
    Int32 perm;
    FAIL_RETURN(mContext->CheckCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::WRITE_SECURE_SETTINGS, &perm));
    if (perm != IPackageManager::PERMISSION_GRANTED) {
        Slogger::E(TAG, "Must hold permission %s", Elastos::Droid::Manifest::permission::WRITE_SECURE_SETTINGS.string());
        return E_SECURITY_EXCEPTION;
    }

    if (displayId != IDisplay::DEFAULT_DISPLAY) {
        Slogger::E(TAG, "Can only set the default display");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    Int64 ident = Binder::ClearCallingIdentity();
    // try {
    synchronized (mWindowMapLock) {
        AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
        if (displayContent != NULL) {
            SetForcedDisplayDensityLocked(displayContent, density);
            AutoPtr<IContentResolver> cr;
            mContext->GetContentResolver((IContentResolver**)&cr);
            Boolean result;
            Settings::Global::PutString(cr, ISettingsGlobal::DISPLAY_DENSITY_FORCED, StringUtils::ToString(density), &result);
        }
    }
    // } finally {
    //     Binder.restoreCallingIdentity(ident);
    // }
    Binder::RestoreCallingIdentity(ident);
    return NOERROR;
}

void CWindowManagerService::SetForcedDisplayDensityLocked(
    /* [in] */ DisplayContent* displayContent,
    /* [in] */ Int32 density)
{
    Slogger::I(TAG, "Using new display density: %d", density);

    {
        AutoLock lock(displayContent->mDisplaySizeLock);
        displayContent->mBaseDisplayDensity = density;
    }
    ReconfigureDisplayLocked(displayContent);
}

ECode CWindowManagerService::ClearForcedDisplayDensity(
    /* [in] */ Int32 displayId)
{
    Int32 perm;
    FAIL_RETURN(mContext->CheckCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::WRITE_SECURE_SETTINGS, &perm));
    if (perm != IPackageManager::PERMISSION_GRANTED) {
        Slogger::E(TAG, "Must hold permission %s", Elastos::Droid::Manifest::permission::WRITE_SECURE_SETTINGS.string());
        return E_SECURITY_EXCEPTION;
    }

    if (displayId != IDisplay::DEFAULT_DISPLAY) {
        Slogger::E(TAG, "Can only set the default display");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    Int64 ident = Binder::ClearCallingIdentity();
    // try {
    synchronized (mWindowMapLock) {
        AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
        if (displayContent != NULL) {
            SetForcedDisplayDensityLocked(displayContent, displayContent->mInitialDisplayDensity);
            AutoPtr<IContentResolver> cr;
            mContext->GetContentResolver((IContentResolver**)&cr);
            Boolean result;
            Settings::Global::PutString(cr, ISettingsGlobal::DISPLAY_DENSITY_FORCED, String(""), &result);
        }
    }
    // } finally {
    //     Binder.restoreCallingIdentity(ident);
    // }
    Binder::RestoreCallingIdentity(ident);
    return NOERROR;
}

void CWindowManagerService::ReconfigureDisplayLocked(
    /* [in] */ DisplayContent* displayContent)
{
    // TODO: Multidisplay: for now only use with default display.
    ConfigureDisplayPolicyLocked(displayContent);
    displayContent->mLayoutNeeded = TRUE;

    Boolean configChanged = UpdateOrientationFromAppTokensLocked(FALSE);
    mTempConfiguration->SetToDefaults();
    Float scale;
    mCurConfiguration->GetFontScale(&scale);
    mTempConfiguration->SetFontScale(scale);
    if (ComputeScreenConfigurationLocked(mTempConfiguration)) {
        Int32 result;
        mCurConfiguration->Diff(mTempConfiguration, &result);
        if (result != 0) {
            configChanged = TRUE;
        }
    }

    if (configChanged) {
        mWaitingForConfig = TRUE;
        StartFreezingDisplayLocked(FALSE, 0, 0);
        Boolean result;
        mH->SendEmptyMessage(H::SEND_NEW_CONFIGURATION, &result);
    }

    PerformLayoutAndPlaceSurfacesLocked();
}

void CWindowManagerService::ConfigureDisplayPolicyLocked(
    /* [in] */ DisplayContent* displayContent)
{
    AutoPtr<IDisplay> d = displayContent->GetDisplay();
    mPolicy->SetInitialDisplaySize(d, displayContent->mBaseDisplayWidth,
            displayContent->mBaseDisplayHeight, displayContent->mBaseDisplayDensity);

    AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
    Int32 left, top, right, bottom;
    displayInfo->GetOverscanLeft(&left);
    displayInfo->GetOverscanTop(&top);
    displayInfo->GetOverscanRight(&right);
    displayInfo->GetOverscanBottom(&bottom);
    mPolicy->SetDisplayOverscan(d, left, top, right, bottom);
}

ECode CWindowManagerService::SetOverscan(
    /* [in] */ Int32 displayId,
    /* [in] */ Int32 left,
    /* [in] */ Int32 top,
    /* [in] */ Int32 right,
    /* [in] */ Int32 bottom)
{
    Int32 perm;
    FAIL_RETURN(mContext->CheckCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::WRITE_SECURE_SETTINGS, &perm));
    if (perm != IPackageManager::PERMISSION_GRANTED) {
        Slogger::E(TAG, "Must hold permission %s", Elastos::Droid::Manifest::permission::WRITE_SECURE_SETTINGS.string());
        return E_SECURITY_EXCEPTION;
    }
    Int64 ident = Binder::ClearCallingIdentity();
    // try {
    synchronized (mWindowMapLock) {
        AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
        if (displayContent != NULL) {
            SetOverscanLocked(displayContent, left, top, right, bottom);
        }
    }
    // } finally {
    //     Binder.restoreCallingIdentity(ident);
    // }
    Binder::RestoreCallingIdentity(ident);
    return NOERROR;
}

void CWindowManagerService::SetOverscanLocked(
    /* [in] */ DisplayContent* displayContent,
    /* [in] */ Int32 left,
    /* [in] */ Int32 top,
    /* [in] */ Int32 right,
    /* [in] */ Int32 bottom)
{
    AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
    {
        AutoLock lock(displayContent->mDisplaySizeLock);
        displayInfo->SetOverscanLeft(left);
        displayInfo->SetOverscanTop(top);
        displayInfo->SetOverscanRight(right);
        displayInfo->SetOverscanBottom(bottom);
    }

    String name;
    displayInfo->GetName(&name);
    mDisplaySettings->SetOverscanLocked(name, left, top, right, bottom);
    mDisplaySettings->WriteSettingsLocked();

    ReconfigureDisplayLocked(displayContent);
}

// -------------------------------------------------------------
// Internals
// -------------------------------------------------------------
ECode CWindowManagerService::WindowForClientLocked(
    /* [in] */ CSession* session,
    /* [in] */ IIWindow* client,
    /* [in] */ Boolean throwOnError,
    /* [out] */ WindowState** win)
{
    return WindowForClientLocked(session, IBinder::Probe(client), throwOnError, win);
}

ECode CWindowManagerService::WindowForClientLocked(
    /* [in] */ CSession* session,
    /* [in] */ IBinder* client,
    /* [in] */ Boolean throwOnError,
    /* [out] */ WindowState** w)
{
    VALIDATE_NOT_NULL(w)
    *w = NULL;
    AutoPtr<WindowState> win;
    HashMap<AutoPtr<IBinder>, AutoPtr<WindowState> >::Iterator it = mWindowMap.Find(client);
    if (it != mWindowMap.End()) {
        win = it->mSecond;
    }

    // if (localLOGV) Slog.v(
    //         TAG, "Looking up client " + client + ": " + win);
    if (win == NULL) {
        // RuntimeException ex = new IllegalArgumentException(
        //         "Requested window " + client + " does not exist");
        if (throwOnError) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }
        return NOERROR;
    }
    if (session != NULL && win->mSession.Get() != session) {
        // RuntimeException ex = new IllegalArgumentException(
        //         "Requested window " + client + " is in session " +
        //         win.mSession + ", not " + session);
        if (throwOnError) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }
        Slogger::W(TAG, "Failed looking up window");
        return NOERROR;
    }

    *w = win;
    REFCOUNT_ADD(*w)
    return NOERROR;
}

void CWindowManagerService::RebuildAppWindowListLocked()
{
    RebuildAppWindowListLocked(GetDefaultDisplayContentLocked());
}

void CWindowManagerService::RebuildAppWindowListLocked(
    /* [in] */ DisplayContent* displayContent)
{
    AutoPtr<WindowList> windows = displayContent->GetWindowList();
    Int32 NW = windows->GetSize();
    WindowList::Iterator lastBelow = windows->End();
    Int32 numRemoved = 0;

    if (mRebuildTmp->GetLength() < NW) {
        mRebuildTmp = ArrayOf<WindowState*>::Alloc(NW + 10);
    }

    // First remove all existing app windows.
    Int32 indexOfIt = 0;
    Int32 indexofLastBelow;
    WindowList::Iterator it = windows->Begin();
    while (it != windows->End()) {
        AutoPtr<WindowState> w = *it;
        if (w->mAppToken != NULL) {
            w->mRebuilding = TRUE;
            mRebuildTmp->Set(numRemoved, w);
            it = windows->Erase(it);
            mWindowsChanged = TRUE;
            if (DEBUG_WINDOW_MOVEMENT) Slogger::V(TAG, "Rebuild removing window: %p", w.Get());
            numRemoved++;
            continue;
        }
        else if (lastBelow == --WindowList::Iterator(it)) {
            Int32 type;
            w->mAttrs->GetType(&type);
            if (type == IWindowManagerLayoutParams::TYPE_WALLPAPER
                    || type == IWindowManagerLayoutParams::TYPE_UNIVERSE_BACKGROUND) {
                lastBelow = it;
                indexofLastBelow = indexOfIt;
            }
        }
        ++it;
        ++indexOfIt;
    }

    // The wallpaper window(s) typically live at the bottom of the stack,
    // so skip them before adding app tokens.
    lastBelow++;
    it = lastBelow;

    // First add all of the exiting app tokens...  these are no longer
    // in the main app list, but still have windows shown.  We put them
    // in the back because now that the animation is over we no longer
    // will care about them.
    AutoPtr< List<AutoPtr<TaskStack> > > stacks = displayContent->GetStacks();
    List<AutoPtr<TaskStack> >::Iterator stackIt = stacks->Begin();
    for (; stackIt != stacks->End(); ++stackIt) {
        AppTokenList exitingAppTokens = (*stackIt)->mExitingAppTokens;
        AppTokenList::Iterator tokenIt = exitingAppTokens.Begin();
        for (; tokenIt != exitingAppTokens.End(); ++tokenIt) {
            it = ReAddAppWindowsLocked(displayContent, it, *tokenIt);
        }
    }

    // And add in the still active app tokens in Z order.
    for (stackIt = stacks->Begin(); stackIt != stacks->End(); ++stackIt) {
        AutoPtr< List<AutoPtr<Task> > > tasks = (*stackIt)->GetTasks();
        List<AutoPtr<Task> >::Iterator taskIt = tasks->Begin();
        for (; taskIt != tasks->End(); ++taskIt) {
            AppTokenList tokens = (*taskIt)->mAppTokens;
            AppTokenList::Iterator tokenIt = tokens.Begin();
            for (; tokenIt != tokens.End(); ++tokenIt) {
                AutoPtr<AppWindowToken> wtoken = *tokenIt;
                if (wtoken->mDeferRemoval) {
                    continue;
                }
                it = ReAddAppWindowsLocked(displayContent, it, wtoken);
            }
        }
    }

    indexOfIt -= indexofLastBelow;
    if (indexOfIt != numRemoved) {
        Slogger::W(TAG, "On display=%d Rebuild removed %d windows but added %d"
                , displayContent->GetDisplayId(),numRemoved, indexOfIt/*, new RuntimeException("here").fillInStackTrace()*/);
        for (Int32 i = 0; i < numRemoved; i++) {
            AutoPtr<WindowState> ws = (*mRebuildTmp)[i];
            if (ws->mRebuilding) {
                AutoPtr<IStringWriter> sw;
                CStringWriter::New((IStringWriter**)&sw);
                AutoPtr<IFastPrintWriter> fpw;
                CFastPrintWriter::New(IWriter::Probe(sw), FALSE, 1024, (IFastPrintWriter**)&fpw);
                // ws.dump(pw, "", true);
                IFlushable::Probe(fpw)->Flush();
                Slogger::W(TAG, "This window was lost: %s", TO_CSTR(ws));
                Slogger::W(TAG, TO_CSTR(sw));
                ws->mWinAnimator->DestroySurfaceLocked();
            }
        }

        // Slogger::W(TAG, "Current app token list:");
        // DumpAppTokensLocked();
        // Slogger::W(TAG, "Final window list:");
        // DumpWindowsLocked();
    }
}

void CWindowManagerService::AssignLayersLocked(
    /* [in] */ WindowList* windows)
{
    Int32 curBaseLayer = 0;
    Int32 curLayer = 0;
    WindowList::Iterator it;

    // if (DEBUG_LAYERS) Slogger::V(TAG, "Assigning layers based on windows=" + windows,
    //             new RuntimeException("here").fillInStackTrace());

    Boolean anyLayerChanged = FALSE;

    for (it = windows->Begin(); it != windows->End(); it++) {
        AutoPtr<WindowState> w = *it;
        AutoPtr<WindowStateAnimator> winAnimator = w->mWinAnimator;
        Boolean layerChanged = FALSE;
        Int32 oldLayer = w->mLayer;
        if (w->mBaseLayer == curBaseLayer || w->mIsImWindow
                || (it != windows->Begin() && w->mIsWallpaper)) {
            curLayer += WINDOW_LAYER_MULTIPLIER;
            w->mLayer = curLayer;
        }
        else {
            curBaseLayer = curLayer = w->mBaseLayer;
            w->mLayer = curLayer;
        }
        if (w->mLayer != oldLayer) {
            layerChanged = TRUE;
            anyLayerChanged = TRUE;
        }
        AutoPtr<AppWindowToken> wtoken = w->mAppToken;
        oldLayer = winAnimator->mAnimLayer;
        if (w->mTargetAppToken != NULL) {
            winAnimator->mAnimLayer =
                    w->mLayer + w->mTargetAppToken->mAppAnimator->mAnimLayerAdjustment;
        }
        else if (wtoken != NULL) {
            winAnimator->mAnimLayer =
                    w->mLayer + wtoken->mAppAnimator->mAnimLayerAdjustment;
        }
        else {
            winAnimator->mAnimLayer = w->mLayer;
        }
        if (w->mIsImWindow) {
            winAnimator->mAnimLayer += mInputMethodAnimLayerAdjustment;
        }
        else if (w->mIsWallpaper) {
            winAnimator->mAnimLayer += mWallpaperAnimLayerAdjustment;
        }
        if (winAnimator->mAnimLayer != oldLayer) {
            layerChanged = TRUE;
            anyLayerChanged = TRUE;
        }
        AutoPtr<TaskStack> stack = w->GetStack();
        if (layerChanged && stack != NULL && stack->IsDimming(winAnimator)) {
            // Force an animation pass just to update the mDimLayer layer.
            ScheduleAnimationLocked();
        }
        if (DEBUG_LAYERS) {
            String str = String("");
            if (wtoken != NULL) {
                str = String(" mAppLayer=") + StringUtils::ToString(wtoken->mAppAnimator->mAnimLayerAdjustment);
            }
            Slogger::V(TAG, "Assign layer %p: mBase=%d mLayer=%d %s =mAnimLayer="
                    , w.Get(), w->mBaseLayer, w->mLayer, str.string(), winAnimator->mAnimLayer);
        }

        // System.out.println(
        //     "Assigned layer " + curLayer + " to " + w.mClient.asBinder());
    }

    //TODO (multidisplay): Magnification is supported only for the default display.
    if (mAccessibilityController != NULL && anyLayerChanged
            && (*windows->End())->GetDisplayId() == IDisplay::DEFAULT_DISPLAY) {
        mAccessibilityController->OnWindowLayersChangedLocked();
    }
}

void CWindowManagerService::PerformLayoutAndPlaceSurfacesLocked()
{
    Int32 loopCount = 6;
    do {
        mTraversalScheduled = FALSE;
        PerformLayoutAndPlaceSurfacesLockedLoop();
        mH->RemoveMessages(H::DO_TRAVERSAL);
        loopCount--;
    }
    while (mTraversalScheduled && loopCount > 0);
    mInnerFields->mWallpaperActionPending = FALSE;
}

void CWindowManagerService::PerformLayoutAndPlaceSurfacesLockedLoop()
{
    if (mInLayout) {
        // if (DEBUG) {
        //     throw new RuntimeException("Recursive call!");
        // }
        // Slog.w(TAG, "performLayoutAndPlaceSurfacesLocked called while in layout. Callers="
        //         + Debug.getCallers(3));
        return;
    }
    // TODO: CActivityManagerService::UpdateConfigurationLocked ScheduleConfigurationChanged
    //
    if (mWaitingForConfig) {
        // Our configuration has changed (most likely rotation), but we
        // don't yet have the complete configuration to report to
        // applications.  Don't do any window layout until we have it.
        return;
    }

    if (!mDisplayReady) {
        // Not yet initialized, nothing to do.
        return;
    }

    // AutoPtr<ITrace> trace;
    // ASSERT_SUCCEEDED(CTrace::AcquireSingleton((ITrace**)&trace));
    // trace->TraceBegin(ITrace::TRACE_TAG_WINDOW_MANAGER, String("wmLayout"));
    mInLayout = TRUE;
    Boolean recoveringMemory = FALSE;

    // try {
    if (mForceRemoves != NULL) {
        recoveringMemory = TRUE;
        // Wait a little it for things to settle down, and off we go.
        List< AutoPtr<WindowState> >::Iterator it;
        for (it = mForceRemoves->Begin(); it != mForceRemoves->End(); ++it) {
            AutoPtr<WindowState> ws = *it;
            Slogger::I(TAG, "Force removing: %p", ws.Get());
            RemoveWindowInnerLocked(ws->mSession, ws);
        }
        mForceRemoves = NULL;
        Slogger::W(TAG, "Due to memory failure, waiting a bit for next layout");
        Object tmp;
        synchronized (tmp) {
            // try {
            tmp.Wait(250);
            // } catch (InterruptedException e) {
            // }
        }
    }
    // catch (RuntimeException e) {
    //     Log.wtf(TAG, "Unhandled exception while force removing for memory", e);
    // }

    // try {
    PerformLayoutAndPlaceSurfacesLockedInner(recoveringMemory);

    mInLayout = FALSE;

    if (NeedsLayout()) {
        if (++mLayoutRepeatCount < 6) {
            RequestTraversalLocked();
        }
        else {
            Slogger::E(TAG, "Performed 6 layouts in a row. Skipping");
            mLayoutRepeatCount = 0;
        }
    }
    else {
        mLayoutRepeatCount = 0;
    }

    if (mWindowsChanged && mWindowChangeListeners.IsEmpty()) {
        mH->RemoveMessages(H::REPORT_WINDOWS_CHANGE);
        Boolean result;
        mH->SendEmptyMessage(H::REPORT_WINDOWS_CHANGE, &result);
    }
    // } catch (RuntimeException e) {
    //     mInLayout = false;
    //     Log.wtf(TAG, "Unhandled exception while laying out windows", e);
    // }

    // trace->TraceEnd(ITrace::TRACE_TAG_WINDOW_MANAGER);
}

void CWindowManagerService::PerformLayoutLockedInner(
    /* [in] */ DisplayContent* displayContent,
    /* [in] */ Boolean initial,
    /* [in] */ Boolean updateInputWindows)
{
    if (!displayContent->mLayoutNeeded) {
        return;
    }
    displayContent->mLayoutNeeded = FALSE;
    AutoPtr<WindowList> windows = displayContent->GetWindowList();
    Boolean isDefaultDisplay = displayContent->mIsDefaultDisplay;

    AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
    Int32 dw, dh;
    displayInfo->GetLogicalWidth(&dw);
    displayInfo->GetLogicalHeight(&dh);

    List< AutoPtr<FakeWindowImpl> >::Iterator fwIt;
    for (fwIt = mFakeWindows.Begin(); fwIt != mFakeWindows.End(); ++fwIt) {
        (*fwIt)->Layout(dw, dh);
    }

    // if (DEBUG_LAYOUT) {
    //     Slog.v(TAG, "-------------------------------------");
    //     Slog.v(TAG, "performLayout: needed="
    //             + displayContent.layoutNeeded + " dw=" + dw + " dh=" + dh);
    // }

    AutoPtr<WindowStateAnimator> universeBackground;

    mPolicy->BeginLayoutLw(isDefaultDisplay, dw, dh, mRotation);
    if (isDefaultDisplay) {
        // Not needed on non-default displays.
        mPolicy->GetSystemDecorLayerLw(&mSystemDecorLayer);
        mScreenRect->Set(0, 0, dw, dh);
    }

    mPolicy->GetContentRectLw(mTmpContentRect);
    displayContent->Resize(mTmpContentRect);

    Int32 seq = mLayoutSeq + 1;
    if (seq < 0) seq = 0;
    mLayoutSeq = seq;

    Boolean behindDream = FALSE;

    // First perform layout of any root windows (not attached
    // to another window).
    WindowList::ReverseIterator topAttached = windows->REnd();
    WindowList::ReverseIterator wRIt;
    for (wRIt = windows->RBegin(); wRIt != windows->REnd(); ++wRIt) {
        AutoPtr<WindowState> win = *wRIt;

        // Don't do layout of a window if it is not visible, or
        // soon won't be visible, to avoid wasting time and funky
        // changes while a window is animating away.
        Boolean result, isGone;
        mPolicy->CanBeForceHidden(win, win->mAttrs, &result);
        win->IsGoneForLayoutLw(&isGone);
        Boolean gone = (behindDream && result) || isGone;

        // if (DEBUG_LAYOUT && !win->mLayoutAttached) {
        //     Slog.v(TAG, "1ST PASS " + win
        //             + ": gone=" + gone + " mHaveFrame=" + win.mHaveFrame
        //             + " mLayoutAttached=" + win.mLayoutAttached
        //             + " screen changed=" + win.isConfigChanged());
        //     final AppWindowToken atoken = win.mAppToken;
        //     if (gone) Slog.v(TAG, "  GONE: mViewVisibility="
        //             + win.mViewVisibility + " mRelayoutCalled="
        //             + win.mRelayoutCalled + " hidden="
        //             + win.mRootToken.hidden + " hiddenRequested="
        //             + (atoken != null && atoken.hiddenRequested)
        //             + " mAttachedHidden=" + win.mAttachedHidden);
        //     else Slog.v(TAG, "  VIS: mViewVisibility="
        //             + win.mViewVisibility + " mRelayoutCalled="
        //             + win.mRelayoutCalled + " hidden="
        //             + win.mRootToken.hidden + " hiddenRequested="
        //             + (atoken != null && atoken.hiddenRequested)
        //             + " mAttachedHidden=" + win.mAttachedHidden);
        // }

        // If this view is GONE, then skip it -- keep the current
        // frame, and let the caller know so they can ignore it
        // if they want.  (We do the normal layout for INVISIBLE
        // windows, since that means "perform layout as normal,
        // just don't display").
        Int32 privateFlags, type;
        if (!gone || !win->mHaveFrame || win->mLayoutNeeded
                || ((win->IsConfigChanged() || win->SetInsetsChanged()) &&
                        ((win->mAttrs->GetPrivateFlags(&privateFlags), (privateFlags & IWindowManagerLayoutParams::PRIVATE_FLAG_KEYGUARD) != 0) ||
                        (win->mAppToken != NULL && win->mAppToken->mLayoutConfigChanges)))
                || (win->mAttrs->GetType(&type), type == IWindowManagerLayoutParams::TYPE_UNIVERSE_BACKGROUND)) {
            if (!win->mLayoutAttached) {
                if (initial) {
                    //Slog.i(TAG, "Window " + this + " clearing mContentChanged - initial");
                    win->mContentChanged = FALSE;
                }
                if (type == IWindowManagerLayoutParams::TYPE_DREAM) {
                    // Don't layout windows behind a dream, so that if it
                    // does stuff like hide the status bar we won't get a
                    // bad transition when it goes away.
                    behindDream = TRUE;
                }
                win->mLayoutNeeded = FALSE;
                win->Prelayout();
                mPolicy->LayoutWindowLw(win, NULL);
                win->mLayoutSeq = seq;
                // if (DEBUG_LAYOUT) Slog.v(TAG, "  LAYOUT: mFrame="
                //         + win.mFrame + " mContainingFrame="
                //         + win.mContainingFrame + " mDisplayFrame="
                //         + win.mDisplayFrame);
            }
            else {
                if (topAttached == windows->REnd()) topAttached = wRIt;
            }
        }
        if (win->mViewVisibility == IView::VISIBLE
                && type == IWindowManagerLayoutParams::TYPE_UNIVERSE_BACKGROUND
                && universeBackground == NULL) {
            universeBackground = win->mWinAnimator;
        }
    }

    if (mAnimator->mUniverseBackground  != universeBackground) {
        mFocusMayChange = TRUE;
        mAnimator->mUniverseBackground = universeBackground;
    }

    Boolean attachedBehindDream = FALSE;

    // Now perform layout of attached windows, which usually
    // depend on the position of the window they are attached to.
    // XXX does not deal with windows that are attached to windows
    // that are themselves attached.
    for (wRIt = topAttached; wRIt != windows->REnd(); ++wRIt) {
        AutoPtr<WindowState> win = *wRIt;
        if (win->mLayoutAttached) {
            if (DEBUG_LAYOUT) {
                Slogger::V(TAG, "2ND PASS %p mHaveFrame=%d mViewVisibility=%d mRelayoutCalled=%d",
                        win.Get(), win->mHaveFrame, win->mViewVisibility, win->mRelayoutCalled);
            }
            // If this view is GONE, then skip it -- keep the current
            // frame, and let the caller know so they can ignore it
            // if they want.  (We do the normal layout for INVISIBLE
            // windows, since that means "perform layout as normal,
            // just don't display").
            Boolean result;
            if (attachedBehindDream &&
                    (mPolicy->CanBeForceHidden(win, win->mAttrs, &result), result)) {
                continue;
            }
            if ((win->mViewVisibility != IView::GONE && win->mRelayoutCalled)
                    || !win->mHaveFrame || win->mLayoutNeeded) {
                if (initial) {
                    // Slogger::I(TAG, "Window %p  clearing mContentChanged - initial", this);
                    win->mContentChanged = FALSE;
                }
                win->mLayoutNeeded = FALSE;
                win->Prelayout();
                mPolicy->LayoutWindowLw(win, win->mAttachedWindow);
                win->mLayoutSeq = seq;
                // if (DEBUG_LAYOUT) Slog.v(TAG, "  LAYOUT: mFrame="
                //         + win.mFrame + " mContainingFrame="
                //         + win.mContainingFrame + " mDisplayFrame="
                //         + win.mDisplayFrame);
            }
        }
        else {
            // Don't layout windows behind a dream, so that if it
            // does stuff like hide the status bar we won't get a
            // bad transition when it goes away.
            Int32 type;
            win->mAttrs->GetType(&type);
            if (type == IWindowManagerLayoutParams::TYPE_DREAM)
                attachedBehindDream = behindDream;
        }
    }

    // Window frames may have changed.  Tell the input dispatcher about it.
    mInputMonitor->SetUpdateInputWindowsNeededLw();
    if (updateInputWindows) {
        mInputMonitor->UpdateInputWindowsLw(FALSE /*force*/);
    }

    mPolicy->FinishLayoutLw();
}

void CWindowManagerService::MakeWindowFreezingScreenIfNeededLocked(
    /* [in] */ WindowState* w)
{
    // If the screen is currently frozen or off, then keep
    // it frozen/off until this window draws at its new
    // orientation.
    if (!OkToDisplay()) {
        if (DEBUG_ORIENTATION) Slogger::V(TAG, "Changing surface while display frozen: %p", w);
        w->mOrientationChanging = TRUE;
        w->mLastFreezeDuration = 0;
        //actions_code(ywwang, start)
        if(mDisplayFrozen && mInnerFields->mOrientationChangeComplete
                && mWindowsFreezingScreen && w != NULL && w->mHasSurface) {
            Int32 frozeTimeSec=(Int32)(SystemClock::GetElapsedRealtime() - mDisplayFreezeTime) / 1000;
            if(frozeTimeSec > 30){
                //oh, we really lockup!
                Slogger::W(TAG, "ywwang, makeWindowFreezingScreenIfNeededLocked error");
                return;
            }
        }
        //actions_code(ywwang, end)

        mInnerFields->mOrientationChangeComplete = FALSE;
        if (!mWindowsFreezingScreen) {
            mWindowsFreezingScreen = TRUE;
            // XXX should probably keep timeout from
            // when we first froze the display.
            mH->RemoveMessages(H::WINDOW_FREEZE_TIMEOUT);
            Boolean result;
            mH->SendEmptyMessageDelayed(H::WINDOW_FREEZE_TIMEOUT, WINDOW_FREEZE_TIMEOUT_DURATION, &result);
        }
    }
}

Int32 CWindowManagerService::HandleAppTransitionReadyLocked(
    /* [in] */ List<AutoPtr<WindowState> >* windows)
{
    Int32 changes = 0;
    Boolean goodToGo = true;

    if (DEBUG_APP_TRANSITIONS) {
        Int32 NN = mOpeningApps.GetSize();
        Slogger::V(TAG, "Checking %d opening apps (frozen=%d timeout=%d)..."
                , NN, mDisplayFrozen, mAppTransition->IsTimeout());
    }

    if (!mDisplayFrozen && !mAppTransition->IsTimeout()) {
        // If the display isn't frozen, wait to do anything until
        // all of the apps are ready.  Otherwise just go because
        // we'll unfreeze the display when everyone is ready.
        List< AutoPtr<AppWindowToken> >::Iterator it;
        for (it = mOpeningApps.Begin(); it != mOpeningApps.End() && goodToGo; ++it) {
            AutoPtr<AppWindowToken> wtoken = *it;
            if (DEBUG_APP_TRANSITIONS) {
                Slogger::V(TAG, "Check opening app=%p: allDrawn=%d startingDisplayed=%d startingMoved=%d"
                    , wtoken.Get(), wtoken->mAllDrawn, wtoken->mStartingDisplayed, wtoken->mStartingMoved);
            }
            if (!wtoken->mAllDrawn && !wtoken->mStartingDisplayed
                    && !wtoken->mStartingMoved) {
                goodToGo = FALSE;
            }
        }
    }
    if (goodToGo) {
        if (DEBUG_APP_TRANSITIONS) Slogger::V(TAG, "**** GOOD TO GO");
        Int32 transit = mAppTransition->GetAppTransition();
        if (mSkipAppTransitionAnimation) {
            transit = AppTransition::TRANSIT_UNSET;
        }
        mAppTransition->GoodToGo();
        mStartingIconInTransition = FALSE;
        mSkipAppTransitionAnimation = FALSE;

        mH->RemoveMessages(H::APP_TRANSITION_TIMEOUT);

        RebuildAppWindowListLocked();

        // if wallpaper is animating in or out set oldWallpaper to null else to wallpaper
        AutoPtr<WindowState> oldWallpaper =
                mWallpaperTarget != NULL && mWallpaperTarget->mWinAnimator->IsAnimating()
                        && !mWallpaperTarget->mWinAnimator->IsDummyAnimation()
                ? NULL : mWallpaperTarget;

        mInnerFields->mWallpaperMayChange = FALSE;

        // The top-most window will supply the layout params,
        // and we will determine it below.
        AutoPtr<IWindowManagerLayoutParams> animLp;
        Int32 bestAnimLayer = -1;
        Boolean fullscreenAnim = FALSE;
        Boolean voiceInteraction = FALSE;

        if (DEBUG_APP_TRANSITIONS) {
            Slogger::V(TAG, "New wallpaper target=%p, oldWallpaper=%p, lower target=%p, upper target=%p"
                    , mWallpaperTarget.Get(), oldWallpaper.Get(), mLowerWallpaperTarget.Get(), mUpperWallpaperTarget.Get());
        }

        Boolean openingAppHasWallpaper = FALSE;
        Boolean closingAppHasWallpaper = FALSE;
        AutoPtr<AppWindowToken> lowerWallpaperAppToken;
        AutoPtr<AppWindowToken> upperWallpaperAppToken;
        if (mLowerWallpaperTarget == NULL) {
            lowerWallpaperAppToken = upperWallpaperAppToken = NULL;
        }
        else {
            lowerWallpaperAppToken = mLowerWallpaperTarget->mAppToken;
            upperWallpaperAppToken = mUpperWallpaperTarget->mAppToken;
        }

        // Do a first pass through the tokens for two
        // things:
        // (1) Determine if both the closing and opening
        // app token sets are wallpaper targets, in which
        // case special animations are needed
        // (since the wallpaper needs to stay static
        // behind them).
        // (2) Find the layout params of the top-most
        // application window in the tokens, which is
        // what will control the animation theme.
        List< AutoPtr<AppWindowToken> >::Iterator it;
        for (it = mClosingApps.Begin(); it != mClosingApps.End(); ++it) {
            AutoPtr<AppWindowToken> wtoken = *it;
            if (wtoken == lowerWallpaperAppToken || wtoken == upperWallpaperAppToken) {
                closingAppHasWallpaper = TRUE;
            }

            voiceInteraction |= wtoken->mVoiceInteraction;

            if (wtoken->mAppFullscreen) {
                AutoPtr<WindowState> ws = wtoken->FindMainWindow();
                if (ws != NULL) {
                    animLp = ws->mAttrs;
                    bestAnimLayer = ws->mLayer;
                    fullscreenAnim = TRUE;
                }
            }
            else if (!fullscreenAnim) {
                AutoPtr<WindowState> ws = wtoken->FindMainWindow();
                if (ws != NULL) {
                    if (ws->mLayer > bestAnimLayer) {
                        animLp = ws->mAttrs;
                        bestAnimLayer = ws->mLayer;
                    }
                }
            }
        }
        for (it = mOpeningApps.Begin(); it != mOpeningApps.End(); ++it) {
            AutoPtr<AppWindowToken> wtoken = *it;
            if (wtoken == lowerWallpaperAppToken || wtoken == upperWallpaperAppToken) {
                openingAppHasWallpaper = TRUE;
            }

            voiceInteraction |= wtoken->mVoiceInteraction;

            if (wtoken->mAppFullscreen) {
                AutoPtr<WindowState> ws = wtoken->FindMainWindow();
                if (ws != NULL) {
                    animLp = ws->mAttrs;
                    bestAnimLayer = ws->mLayer;
                    fullscreenAnim = TRUE;
                }
            }
            else if (!fullscreenAnim) {
                AutoPtr<WindowState> ws = wtoken->FindMainWindow();
                if (ws != NULL) {
                    if (ws->mLayer > bestAnimLayer) {
                        animLp = ws->mAttrs;
                        bestAnimLayer = ws->mLayer;
                    }
                }
            }
        }

        mAnimateWallpaperWithTarget = FALSE;
        AutoPtr<AppWindowToken> temp;
        if (closingAppHasWallpaper && openingAppHasWallpaper) {
            if (DEBUG_APP_TRANSITIONS) Slogger::V(TAG, "Wallpaper animation!");
            switch (transit) {
                case AppTransition::TRANSIT_ACTIVITY_OPEN:
                case AppTransition::TRANSIT_TASK_OPEN:
                case AppTransition::TRANSIT_TASK_TO_FRONT:
                    transit = AppTransition::TRANSIT_WALLPAPER_INTRA_OPEN;
                    break;
                case AppTransition::TRANSIT_ACTIVITY_CLOSE:
                case AppTransition::TRANSIT_TASK_CLOSE:
                case AppTransition::TRANSIT_TASK_TO_BACK:
                    transit = AppTransition::TRANSIT_WALLPAPER_INTRA_CLOSE;
                    break;
            }
            if (DEBUG_APP_TRANSITIONS) Slogger::V(TAG, "New transit: %d", transit);
        }
        else if ((oldWallpaper != NULL) && mOpeningApps.Begin() != mOpeningApps.End()
                && (Find(mOpeningApps.Begin(), mOpeningApps.End(), temp = oldWallpaper->mAppToken) == mOpeningApps.End())) {
            // We are transitioning from an activity with
            // a wallpaper to one without.
            transit = AppTransition::TRANSIT_WALLPAPER_CLOSE;
            if (DEBUG_APP_TRANSITIONS) Slogger::V(TAG, "New transit away from wallpaper: %d", transit);
        }
        else if (mWallpaperTarget != NULL) {
            Boolean isVisible;
            if (mWallpaperTarget->IsVisibleLw(&isVisible), isVisible) {
                // We are transitioning from an activity without
                // a wallpaper to now showing the wallpaper
                transit = AppTransition::TRANSIT_WALLPAPER_OPEN;
                if (DEBUG_APP_TRANSITIONS) Slogger::V(TAG, "New transit into wallpaper: %d", transit);
            }
        }
        else {
            mAnimateWallpaperWithTarget = TRUE;
        }

        // If all closing windows are obscured, then there is
        // no need to do an animation.  This is the case, for
        // example, when this transition is being done behind
        // the lock screen.
        Boolean allowed;
        mPolicy->AllowAppAnimationsLw(&allowed);
        if (!allowed) {
            if (DEBUG_APP_TRANSITIONS) Slogger::V(TAG, "Animations disallowed by keyguard or dream.");
            animLp = NULL;
        }

        AutoPtr<AppWindowToken> topOpeningApp;
        AutoPtr<AppWindowToken> topClosingApp;
        Int32 topOpeningLayer = 0;
        Int32 topClosingLayer = 0;

        for (it = mOpeningApps.Begin(); it != mOpeningApps.End(); ++it) {
            AutoPtr<AppWindowToken> wtoken = *it;
            AutoPtr<AppWindowAnimator> appAnimator = wtoken->mAppAnimator;
            if (DEBUG_APP_TRANSITIONS) Slogger::V(TAG, "Now opening app %p", wtoken.Get());
            appAnimator->ClearThumbnail();
            appAnimator->mAnimation = NULL;
            wtoken->mInPendingTransaction = FALSE;
            SetTokenVisibilityLocked(wtoken, animLp, TRUE, transit, FALSE, voiceInteraction);
            wtoken->UpdateReportedVisibilityLocked();
            wtoken->mWaitingToShow = FALSE;

            appAnimator->mAllAppWinAnimators.Clear();
            List< AutoPtr<WindowState> >::Iterator appWIt = wtoken->mAllAppWindows.Begin();
            for (; appWIt != wtoken->mAllAppWindows.End(); ++appWIt) {
                appAnimator->mAllAppWinAnimators.PushBack((*appWIt)->mWinAnimator);
            }
            mAnimator->mAnimating |= appAnimator->ShowAllWindowsLocked();

            if (animLp != NULL) {
                Int32 layer = -1;
                List< AutoPtr<WindowState> >::Iterator tokenWIt = wtoken->mWindows.Begin();
                for (; tokenWIt != wtoken->mWindows.End(); ++tokenWIt) {
                    AutoPtr<WindowState> win = *tokenWIt;
                    if (win->mWinAnimator->mAnimLayer > layer) {
                        layer = win->mWinAnimator->mAnimLayer;
                    }
                }
                if (topOpeningApp == NULL || layer > topOpeningLayer) {
                    topOpeningApp = wtoken;
                    topOpeningLayer = layer;
                }
            }
        }
        for (it = mClosingApps.Begin(); it != mClosingApps.End(); ++it) {
            AutoPtr<AppWindowToken> wtoken = *it;
            AutoPtr<AppWindowAnimator> appAnimator = wtoken->mAppAnimator;
            if (DEBUG_APP_TRANSITIONS) Slogger::V(TAG, "Now closing app %p", wtoken.Get());
            appAnimator->ClearThumbnail();
            appAnimator->mAnimation = NULL;
            wtoken->mInPendingTransaction = FALSE;
            wtoken->mAppAnimator->mAnimation = NULL;
            SetTokenVisibilityLocked(wtoken, animLp, FALSE, transit, FALSE, voiceInteraction);
            wtoken->UpdateReportedVisibilityLocked();
            wtoken->mWaitingToHide = FALSE;
            // Force the allDrawn flag, because we want to start
            // this guy's animations regardless of whether it's
            // gotten drawn.
            wtoken->mAllDrawn = TRUE;
            wtoken->mDeferClearAllDrawn = FALSE;
            // Ensure that apps that are mid-starting are also scheduled to have their
            // starting windows removed after the animation is complete
            if (wtoken->mStartingWindow != NULL && !wtoken->mStartingWindow->mExiting) {
                ScheduleRemoveStartingWindowLocked(wtoken);
            }

            if (animLp != NULL) {
                Int32 layer = -1;
                WindowList::Iterator winIt = wtoken->mWindows.Begin();
                for (; winIt != wtoken->mWindows.End(); ++winIt) {
                    AutoPtr<WindowState> win = *winIt;
                    if (win->mWinAnimator->mAnimLayer > layer) {
                        layer = win->mWinAnimator->mAnimLayer;
                    }
                }
                if (topClosingApp == NULL || layer > topClosingLayer) {
                    topClosingApp = wtoken;
                    topClosingLayer = layer;
                }
            }
        }

        AutoPtr<AppWindowAnimator> openingAppAnimator
                = (topOpeningApp == NULL) ? NULL : topOpeningApp->mAppAnimator;
        AutoPtr<AppWindowAnimator> closingAppAnimator
                = (topClosingApp == NULL) ? NULL : topClosingApp->mAppAnimator;
        AutoPtr<IBitmap> nextAppTransitionThumbnail = mAppTransition->GetNextAppTransitionThumbnail();
        BitmapConfig config;
        if (nextAppTransitionThumbnail != NULL
                && openingAppAnimator != NULL && openingAppAnimator->mAnimation != NULL &&
                (nextAppTransitionThumbnail->GetConfig(&config), config != BitmapConfig_ALPHA_8)) {
            // This thumbnail animation is very special, we need to have
            // an extra surface with the thumbnail included with the animation.
            AutoPtr<IRect> dirty;
            Int32 width, height;
            nextAppTransitionThumbnail->GetWidth(&width);
            nextAppTransitionThumbnail->GetHeight(&height);
            CRect::New(0, 0, width, height, (IRect**)&dirty);
            // try {
            // TODO(multi-display): support other displays
            AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();
            AutoPtr<IDisplay> display = displayContent->GetDisplay();
            AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();

            // Create a new surface for the thumbnail
            Int32 rectW, rectH;
            dirty->GetWidth(&rectW);
            dirty->GetHeight(&rectH);
            AutoPtr<ISurfaceControl> surfaceControl;
            CSurfaceControl::New(mFxSession, String("thumbnail anim"), rectW, rectH,
                    IPixelFormat::TRANSLUCENT, ISurfaceControl::HIDDEN, (ISurfaceControl**)&surfaceControl);
            Int32 strack;
            display->GetLayerStack(&strack);
            surfaceControl->SetLayerStack(strack);
            if (SHOW_TRANSACTIONS) {
                Slogger::I(TAG, "  THUMBNAIL %p: CREATE", surfaceControl.Get());
            }

            // Draw the thumbnail onto the surface
            AutoPtr<ISurface> drawSurface;
            CSurface::New((ISurface**)&drawSurface);
            drawSurface->CopyFrom(surfaceControl);
            AutoPtr<ICanvas> c;
            drawSurface->LockCanvas(dirty, (ICanvas**)&c);
            c->DrawBitmap(nextAppTransitionThumbnail, 0.0, 0.0, NULL);
            drawSurface->UnlockCanvasAndPost(c);
            drawSurface->Release();

            // Get the thumbnail animation
            AutoPtr<IAnimation> anim;
            if (mAppTransition->IsNextThumbnailTransitionAspectScaled()) {
                // For the new aspect-scaled transition, we want it to always show
                // above the animating opening/closing window, and we want to
                // synchronize its thumbnail surface with the surface for the
                // open/close animation (only on the way down)
                Int32 appW, appH, logicalW;
                displayInfo->GetAppWidth(&appW);
                displayInfo->GetAppHeight(&appH);
                displayInfo->GetLogicalWidth(&logicalW);
                anim = mAppTransition->CreateThumbnailAspectScaleAnimationLocked(
                        appW, appH, logicalW, transit);
                openingAppAnimator->mThumbnailForceAboveLayer = Elastos::Core::Math::Max(
                        topOpeningLayer, topClosingLayer);
                openingAppAnimator->mDeferThumbnailDestruction = !mAppTransition->IsNextThumbnailTransitionScaleUp();
                if (openingAppAnimator->mDeferThumbnailDestruction) {
                    if (closingAppAnimator != NULL && closingAppAnimator->mAnimation != NULL) {
                        closingAppAnimator->mDeferredThumbnail = surfaceControl;
                    }
                }
            }
            else {
                Int32 appW, appH;
                displayInfo->GetAppWidth(&appW);
                displayInfo->GetAppHeight(&appH);
                anim = mAppTransition->CreateThumbnailScaleAnimationLocked(appW, appH, transit);
            }
            anim->RestrictDuration(MAX_ANIMATION_DURATION);
            anim->ScaleCurrentDuration(GetTransitionAnimationScaleLocked());
            openingAppAnimator->mThumbnail = surfaceControl;
            openingAppAnimator->mThumbnailLayer = topOpeningLayer;
            openingAppAnimator->mThumbnailAnimation = anim;
            openingAppAnimator->mThumbnailX = mAppTransition->GetStartingX();
            openingAppAnimator->mThumbnailY = mAppTransition->GetStartingY();
            // } catch (Surface.OutOfResourcesException e) {
            //     Slog.e(TAG, "Can't allocate thumbnail surface w=" + dirty.width()
            //             + " h=" + dirty.height(), e);
            //     topOpeningApp.mAppAnimator.clearThumbnail();
            // }
        }

        mAppTransition->PostAnimationCallback();
        mAppTransition->Clear();

        mOpeningApps.Clear();
        mClosingApps.Clear();

        // This has changed the visibility of windows, so perform
        // a new layout to get them all up-to-date.
        changes |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_LAYOUT
                | IWindowManagerPolicy::FINISH_LAYOUT_REDO_CONFIG;
        GetDefaultDisplayContentLocked()->mLayoutNeeded = TRUE;

        // TODO(multidisplay): IMEs are only supported on the default display.
        if (windows == GetDefaultWindowListLocked()
                && !MoveInputMethodWindowsIfNeededLocked(TRUE)) {
            AssignLayersLocked(windows);
        }
        UpdateFocusedWindowLocked(UPDATE_FOCUS_PLACING_SURFACES, TRUE /*updateInputWindows*/);
        mFocusMayChange = FALSE;
    }

    return changes;
}

Int32 CWindowManagerService::HandleAnimatingStoppedAndTransitionLocked()
{
    Int32 changes = 0;

    mAppTransition->SetIdle();
    // Restore window app tokens to the ActivityManager views
    AutoPtr< List<AutoPtr<TaskStack> > > stacks = GetDefaultDisplayContentLocked()->GetStacks();
    List<AutoPtr<TaskStack> >::ReverseIterator rit = stacks->RBegin();
    for (; rit != stacks->REnd(); ++rit) {
        AutoPtr< List<AutoPtr<Task> > > tasks = (*rit)->GetTasks();
        List<AutoPtr<Task> >::ReverseIterator taskRit = tasks->RBegin();
        for (; taskRit != tasks->REnd(); ++taskRit) {
            AppTokenList tokens = (*taskRit)->mAppTokens;
            AppTokenList::ReverseIterator tokenRit = tokens.RBegin();
            for (; tokenRit != tokens.REnd(); ++tokenRit) {
                (*tokenRit)->mSendingToBottom = FALSE;
            }
        }
    }
    RebuildAppWindowListLocked();

    changes |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_LAYOUT;
    if (DEBUG_WALLPAPER_LIGHT) Slogger::V(TAG, "Wallpaper layer changed: assigning layers + relayout");
    MoveInputMethodWindowsIfNeededLocked(TRUE);
    mInnerFields->mWallpaperMayChange = TRUE;
    // Since the window list has been rebuilt, focus might
    // have to be recomputed since the actual order of windows
    // might have changed again.
    mFocusMayChange = TRUE;

    return changes;
}

void CWindowManagerService::UpdateResizingWindows(
    /* [in] */ WindowState* w)
{
    AutoPtr<WindowStateAnimator> winAnimator = w->mWinAnimator;
    if (w->mHasSurface && w->mLayoutSeq == mLayoutSeq) {
        w->SetInsetsChanged();
        Boolean configChanged = w->IsConfigChanged();
        if (DEBUG_CONFIGURATION && configChanged) {
            Slogger::V(TAG, "Win %p config changed: %p", w, mCurConfiguration.Get());
        }
        if (localLOGV) {
            Slogger::V(TAG, "Resizing %p: configChanged=%d last=%p frame=%p"
                    , w, configChanged, w->mLastFrame.Get(), w->mFrame.Get());
        }
        w->mLastFrame->Set(w->mFrame);
        if (w->mContentInsetsChanged
                || w->mVisibleInsetsChanged
                || winAnimator->mSurfaceResized
                || configChanged) {
            if (DEBUG_RESIZE || DEBUG_ORIENTATION) {
                String str1, str2, str3;
                w->mContentInsets->ToShortString(&str1);
                w->mVisibleInsets->ToShortString(&str2);
                w->mStableInsets->ToShortString(&str3);
                Slogger::V(TAG, "Resize reasons for w=%p:  contentInsetsChanged=%d %s visibleInsetsChanged=%d %s stableInsetsChanged=%d %s surfaceResized=%d configChanged=%d"
                        , w, w->mContentInsetsChanged, str1.string(), w->mVisibleInsetsChanged, str2.string()
                        , w->mStableInsetsChanged, str3.string(), winAnimator->mSurfaceResized, configChanged);
            }

            w->mLastOverscanInsets->Set(w->mOverscanInsets);
            w->mLastContentInsets->Set(w->mContentInsets);
            w->mLastVisibleInsets->Set(w->mVisibleInsets);
            w->mLastStableInsets->Set(w->mStableInsets);
            MakeWindowFreezingScreenIfNeededLocked(w);
            // If the orientation is changing, then we need to
            // hold off on unfreezing the display until this
            // window has been redrawn; to do that, we need
            // to go through the process of getting informed
            // by the application when it has finished drawing.
            if (w->mOrientationChanging) {
                if (DEBUG_SURFACE_TRACE || DEBUG_ANIM || DEBUG_ORIENTATION) {
                    Slogger::V(TAG, "Orientation start waiting for draw mDrawState=DRAW_PENDING in %p, surface %p"
                            , w, winAnimator->mSurfaceControl.Get());
                }
                winAnimator->mDrawState = WindowStateAnimator::DRAW_PENDING;
                if (w->mAppToken != NULL) {
                    w->mAppToken->mAllDrawn = FALSE;
                    w->mAppToken->mDeferClearAllDrawn = FALSE;
                }
            }
            WindowList::Iterator it;
            for (it = mResizingWindows.Begin(); it != mResizingWindows.End(); ++it) {
                if ((*it).Get() == w) break;
            }
            if (it == mResizingWindows.End()) {
                if (DEBUG_RESIZE || DEBUG_ORIENTATION) {
                    Slogger::V(TAG, "Resizing window %p to %dx%d", w, winAnimator->mSurfaceW, winAnimator->mSurfaceH);
                }
                mResizingWindows.PushBack(w);
            }
        }
        else if (w->mOrientationChanging) {
            if (w->IsDrawnLw()) {
                if (DEBUG_ORIENTATION) {
                    Slogger::V(TAG, "Orientation not waiting for draw in %p, surface %p"
                            , w, winAnimator->mSurfaceControl.Get());
                }
                w->mOrientationChanging = FALSE;
                w->mLastFreezeDuration = (Int32)(SystemClock::GetElapsedRealtime() - mDisplayFreezeTime);
            }
        }
    }
}

void CWindowManagerService::HandleNotObscuredLocked(
    /* [in] */ WindowState* w,
    /* [in] */ Int64 currentTime,
    /* [in] */ Int32 innerDw,
    /* [in] */ Int32 innerDh)
{
    AutoPtr<IWindowManagerLayoutParams> attrs = w->mAttrs;
    Int32 attrFlags;
    attrs->GetFlags(&attrFlags);
    Boolean canBeSeen;
    w->IsDisplayedLw(&canBeSeen);
    Boolean opaqueDrawn = canBeSeen && w->IsOpaqueDrawn();

    if (opaqueDrawn && w->IsFullscreen(innerDw, innerDh)) {
        // This window completely covers everything behind it,
        // so we want to leave all of them as undimmed (for
        // performance reasons).
        mInnerFields->mObscured = TRUE;
    }

    if (w->mHasSurface) {
        if ((attrFlags & IWindowManagerLayoutParams::FLAG_KEEP_SCREEN_ON) != 0) {
            mInnerFields->mHoldScreen = w->mSession;
        }
        Float screenbrightness = 0;
        if (!mInnerFields->mSyswin
                && (attrs->GetScreenBrightness(&screenbrightness), screenbrightness >= 0)
                && mInnerFields->mScreenBrightness < 0) {
            mInnerFields->mScreenBrightness = screenbrightness;
        }
        Float buttonBrightness = 0;
        if (!mInnerFields->mSyswin
                && (attrs->GetButtonBrightness(&buttonBrightness), buttonBrightness >= 0)
                && mInnerFields->mButtonBrightness < 0) {
            mInnerFields->mButtonBrightness = buttonBrightness;
        }
        Int32 timeout = 0;
        if (!mInnerFields->mSyswin
                && (attrs->GetUserActivityTimeout(&timeout), timeout >= 0)
                && mInnerFields->mUserActivityTimeout < 0) {
            mInnerFields->mUserActivityTimeout = timeout;
        }

        Int32 type, privateFlags;
        attrs->GetType(&type);
        if (canBeSeen
                && (type == IWindowManagerLayoutParams::TYPE_SYSTEM_DIALOG
                 || type == IWindowManagerLayoutParams::TYPE_RECENTS_OVERLAY
                 || type == IWindowManagerLayoutParams::TYPE_SYSTEM_ERROR
                 || (attrs->GetPrivateFlags(&privateFlags), (privateFlags & IWindowManagerLayoutParams::PRIVATE_FLAG_KEYGUARD) != 0))) {
            mInnerFields->mSyswin = TRUE;
        }

        if (canBeSeen) {
            // This function assumes that the contents of the default display are
            // processed first before secondary displays.
            AutoPtr<DisplayContent> displayContent = w->GetDisplayContent();
            if (displayContent != NULL && displayContent->mIsDefaultDisplay) {
                // While a dream or keyguard is showing, obscure ordinary application
                // content on secondary displays (by forcibly enabling mirroring unless
                // there is other content we want to show) but still allow opaque
                // keyguard dialogs to be shown.
                if (type == IWindowManagerLayoutParams::TYPE_DREAM
                        || (attrs->GetPrivateFlags(&privateFlags), (privateFlags & IWindowManagerLayoutParams::PRIVATE_FLAG_KEYGUARD) != 0)) {
                    mInnerFields->mObscureApplicationContentOnSecondaryDisplays = TRUE;
                }
                mInnerFields->mDisplayHasContent = TRUE;
            }
            else if (displayContent != NULL &&
                    (!mInnerFields->mObscureApplicationContentOnSecondaryDisplays
                    || (mInnerFields->mObscured && type == IWindowManagerLayoutParams::TYPE_KEYGUARD_DIALOG))) {
                // Allow full screen keyguard presentation dialogs to be seen.
                mInnerFields->mDisplayHasContent = TRUE;
            }
            Float preferredRefreshRate;
            if (mInnerFields->mPreferredRefreshRate == 0
                    && (w->mAttrs->GetPreferredRefreshRate(&preferredRefreshRate), preferredRefreshRate != 0)) {
                mInnerFields->mPreferredRefreshRate = preferredRefreshRate;
            }
        }
    }
}

void CWindowManagerService::HandleFlagDimBehind(
    /* [in] */ WindowState* w)
{
    AutoPtr<IWindowManagerLayoutParams> attrs = w->mAttrs;
    Int32 flags;
    Boolean isDisplayed;
    if ((attrs->GetFlags(&flags), (flags & IWindowManagerLayoutParams::FLAG_DIM_BEHIND) != 0)
            && (w->IsDisplayedLw(&isDisplayed), isDisplayed) && !w->mExiting) {
        AutoPtr<WindowStateAnimator> winAnimator = w->mWinAnimator;
        AutoPtr<TaskStack> stack = w->GetStack();
        if (stack == NULL) {
            return;
        }
        stack->SetDimmingTag();
        if (!stack->IsDimming(winAnimator)) {
            if (localLOGV) Slogger::V(TAG, "Win %p start dimming.", w);
            stack->StartDimmingIfNeeded(winAnimator);
        }
    }
}

void CWindowManagerService::UpdateAllDrawnLocked(
    /* [in] */ DisplayContent* displayContent)
{
    // See if any windows have been drawn, so they (and others
    // associated with them) can now be shown.
    // begin from this
    AutoPtr< List<AutoPtr<TaskStack> > > stacks = displayContent->GetStacks();
    List<AutoPtr<TaskStack> >::ReverseIterator rit = stacks->RBegin();
    for (; rit != stacks->REnd(); ++rit) {
        AutoPtr< List<AutoPtr<Task> > > tasks = (*rit)->GetTasks();
        List<AutoPtr<Task> >::ReverseIterator taskRit = tasks->RBegin();
        for (; taskRit != tasks->REnd(); ++taskRit) {
            AppTokenList tokens = (*taskRit)->mAppTokens;
            AppTokenList::ReverseIterator tokenRit = tokens.RBegin();
            for (; tokenRit != tokens.REnd(); tokenRit++) {
                AutoPtr<AppWindowToken> wtoken = *tokenRit;
                if (!wtoken->mAllDrawn) {
                    Int32 numInteresting = wtoken->mNumInterestingWindows;
                    if (numInteresting > 0 && wtoken->mNumDrawnWindows >= numInteresting) {
                        if (DEBUG_VISIBILITY) {
                            Slogger::V(TAG, "allDrawn: %p interesting=%d drawn="
                                    , wtoken.Get(), numInteresting, wtoken->mNumDrawnWindows);
                        }
                        wtoken->mAllDrawn = TRUE;
                        AutoPtr<IMessage> msg;
                        mH->ObtainMessage(H::NOTIFY_ACTIVITY_DRAWN, wtoken->mToken, (IMessage**)&msg);
                        msg->SendToTarget();
                    }
                }
            }
        }
    }
}

void CWindowManagerService::PerformLayoutAndPlaceSurfacesLockedInner(
    /* [in] */ Boolean recoveringMemory)
{
    if (DEBUG_WINDOW_TRACE) {
        Slogger::V(TAG, "performLayoutAndPlaceSurfacesLockedInner: entry. Called by "
                /*+ Debug.getCallers(3)*/);
    }

    Int64 currentTime = SystemClock::GetUptimeMillis();

    Boolean updateInputWindowsNeeded = FALSE;

    if (mFocusMayChange) {
        mFocusMayChange = FALSE;
        updateInputWindowsNeeded = UpdateFocusedWindowLocked(UPDATE_FOCUS_WILL_PLACE_SURFACES,
                FALSE /*updateInputWindows*/);
    }

    // Initialize state of exiting tokens.
    Int32 numDisplays;
    mDisplayContents->GetSize(&numDisplays);
    for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
        AutoPtr<IInterface> value;
        mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
        AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
        List<AutoPtr<WindowToken> >::ReverseIterator tokenRit = displayContent->mExitingTokens.RBegin();
        for (; tokenRit != displayContent->mExitingTokens.REnd(); ++tokenRit) {
            (*tokenRit)->mHasVisible = FALSE;
        }
    }

    Int32 stackSize;
    mStackIdToStack->GetSize(&stackSize);
    for (Int32 stackNdx = stackSize - 1; stackNdx >= 0; --stackNdx) {
        // Initialize state of exiting applications.
        AutoPtr<IInterface> value;
        mStackIdToStack->ValueAt(stackNdx, (IInterface**)&value);
        AutoPtr<TaskStack> taskStack = (TaskStack*)(IObject*)value.Get();
        AppTokenList exitingAppTokens = taskStack->mExitingAppTokens;
        AppTokenList::ReverseIterator tokenRit = exitingAppTokens.RBegin();
        for (; tokenRit != exitingAppTokens.REnd(); ++tokenRit) {
            (*tokenRit)->mHasVisible = FALSE;
        }
    }

    mInnerFields->mHoldScreen = NULL;
    mInnerFields->mScreenBrightness = -1;
    mInnerFields->mButtonBrightness = -1;
    mInnerFields->mUserActivityTimeout = -1;
    mInnerFields->mObscureApplicationContentOnSecondaryDisplays = FALSE;

    mTransactionSequence++;

    AutoPtr<DisplayContent> defaultDisplay = GetDefaultDisplayContentLocked();
    AutoPtr<IDisplayInfo> defaultInfo = defaultDisplay->GetDisplayInfo();
    Int32 defaultDw, defaultDh;
    defaultInfo->GetLogicalWidth(&defaultDw);
    defaultInfo->GetLogicalHeight(&defaultDh);

    if (SHOW_LIGHT_TRANSACTIONS) Slogger::I(TAG, ">>> OPEN TRANSACTION performLayoutAndPlaceSurfaces");
    AutoPtr<ISurfaceControlHelper> helper;
    CSurfaceControlHelper::AcquireSingleton((ISurfaceControlHelper**)&helper);
    helper->OpenTransaction();

    // try {
    if (mWatermark != NULL) {
        mWatermark->PositionSurface(defaultDw, defaultDh);
    }
    if (mStrictModeFlash != NULL) {
        mStrictModeFlash->PositionSurface(defaultDw, defaultDh);
    }
    if (mCircularDisplayMask != NULL) {
        mCircularDisplayMask->PositionSurface(defaultDw, defaultDh, mRotation);
    }
    if (mEmulatorDisplayOverlay != NULL) {
        mEmulatorDisplayOverlay->PositionSurface(defaultDw, defaultDh, mRotation);
    }

    Boolean focusDisplayed = FALSE;

    for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
        AutoPtr<IInterface> value;
        mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
        AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
        Boolean updateAllDrawn = FALSE;
        AutoPtr<WindowList> windows = displayContent->GetWindowList();
        AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
        Int32 displayId = displayContent->GetDisplayId();
        Int32 dw, dh, innerDw, innerDh;
        displayInfo->GetLogicalWidth(&dw);
        displayInfo->GetLogicalHeight(&dh);
        displayInfo->GetAppWidth(&innerDw);
        displayInfo->GetAppHeight(&innerDh);
        Boolean isDefaultDisplay = (displayId == IDisplay::DEFAULT_DISPLAY);

        // Reset for each display.
        mInnerFields->mDisplayHasContent = FALSE;
        mInnerFields->mPreferredRefreshRate = 0;

        Int32 repeats = 0;
        do {
            repeats++;
            if (repeats > 6) {
                // Slogger::W(TAG, "Animation repeat aborted after too many iterations");
                displayContent->mLayoutNeeded = FALSE;
                break;
            }

            if (DEBUG_LAYOUT_REPEATS) DebugLayoutRepeats(String("On entry to LockedInner"),
                displayContent->mPendingLayoutChanges);

            if ((displayContent->mPendingLayoutChanges &
                    IWindowManagerPolicy::FINISH_LAYOUT_REDO_WALLPAPER) != 0 &&
                    (AdjustWallpaperWindowsLocked() & ADJUST_WALLPAPER_LAYERS_CHANGED) != 0) {
                AssignLayersLocked(windows);
                displayContent->mLayoutNeeded = TRUE;
            }

            if (isDefaultDisplay && (displayContent->mPendingLayoutChanges
                    & IWindowManagerPolicy::FINISH_LAYOUT_REDO_CONFIG) != 0) {
                if (DEBUG_LAYOUT) Slogger::V(TAG, "Computing new config from layout");
                if (UpdateOrientationFromAppTokensLocked(TRUE)) {
                    displayContent->mLayoutNeeded = TRUE;
                    Boolean result;
                    mH->SendEmptyMessage(H::SEND_NEW_CONFIGURATION, &result);
                }
            }

            if ((displayContent->mPendingLayoutChanges
                    & IWindowManagerPolicy::FINISH_LAYOUT_REDO_LAYOUT) != 0) {
                displayContent->mLayoutNeeded = TRUE;
            }

            // FIRST LOOP: Perform a layout, if needed.
            if (repeats < 4) {
                PerformLayoutLockedInner(displayContent, repeats == 1,
                        FALSE /*updateInputWindows*/);
            }
            else {
                Slogger::W(TAG, "Layout repeat skipped after too many iterations");
            }

            // FIRST AND ONE HALF LOOP: Make WindowManagerPolicy think
            // it is animating.
            displayContent->mPendingLayoutChanges = 0;

            StringBuilder sb("loop number ");
            sb += mLayoutRepeatCount;
            if (DEBUG_LAYOUT_REPEATS) DebugLayoutRepeats(sb.ToString(), displayContent->mPendingLayoutChanges);

            if (isDefaultDisplay) {
                mPolicy->BeginPostLayoutPolicyLw(dw, dh);
                List< AutoPtr<WindowState> >::ReverseIterator winRIt = windows->RBegin();
                for (; winRIt != windows->REnd(); ++winRIt) {
                    AutoPtr<WindowState> w = *winRIt;
                    if (w->mHasSurface) {
                        mPolicy->ApplyPostLayoutPolicyLw(w, w->mAttrs);
                    }
                }
                Int32 result;
                mPolicy->FinishPostLayoutPolicyLw(&result);
                displayContent->mPendingLayoutChanges |= result;
                if (DEBUG_LAYOUT_REPEATS) DebugLayoutRepeats(
                    String("after finishPostLayoutPolicyLw"), displayContent->mPendingLayoutChanges);
            }
        }
        while (displayContent->mPendingLayoutChanges != 0);

        mInnerFields->mObscured = FALSE;
        mInnerFields->mSyswin = FALSE;
        displayContent->ResetDimming();

        // Only used if default window
        Boolean someoneLosingFocus = !mLosingFocus.IsEmpty();

        List< AutoPtr<WindowState> >::ReverseIterator winRIt = windows->RBegin();
        for (; winRIt != windows->REnd(); ++winRIt) {
            AutoPtr<WindowState> w = *winRIt;
            AutoPtr<TaskStack> stack = w->GetStack();
            Int32 type;
            if (stack == NULL) {
                AutoPtr<IWindowManagerLayoutParams> attrs;
                w->GetAttrs((IWindowManagerLayoutParams**)&attrs);
                if (attrs->GetType(&type), type != IWindowManagerLayoutParams::TYPE_PRIVATE_PRESENTATION) {
                    continue;
                }
            }

            Boolean obscuredChanged = w->mObscured != mInnerFields->mObscured;

            // Update effect.
            w->mObscured = mInnerFields->mObscured;
            if (!mInnerFields->mObscured) {
                HandleNotObscuredLocked(w, currentTime, innerDw, innerDh);
            }

            if (stack != NULL && !stack->TestDimmingTag()) {
                HandleFlagDimBehind(w);
            }

            Boolean isVisible;
            if (isDefaultDisplay && obscuredChanged && (mWallpaperTarget == w)
                    && (w->IsVisibleLw(&isVisible), isVisible)) {
                // This is the wallpaper target and its obscured state
                // changed... make sure the current wallaper's visibility
                // has been updated accordingly.
                UpdateWallpaperVisibilityLocked();
            }

            AutoPtr<WindowStateAnimator> winAnimator = w->mWinAnimator;

            // If the window has moved due to its containing
            // content frame changing, then we'd like to animate
            // it.
            if (w->mHasSurface && w->ShouldAnimateMove()) {
                // Frame has moved, containing content frame
                // has also moved, and we're not currently animating...
                // let's do something.
                AutoPtr<IAnimation> a;
                AutoPtr<IAnimationUtils> animationUtils;
                CAnimationUtils::AcquireSingleton((IAnimationUtils**)&animationUtils);
                animationUtils->LoadAnimation(mContext,
                        Elastos::Droid::R::anim::window_move_from_decor, (IAnimation**)&a);
                winAnimator->SetAnimation(a);
                Int32 left1, left2;
                w->mLastFrame->GetLeft(&left1);
                w->mFrame->GetLeft(&left2);
                winAnimator->mAnimDw = left1 - left2;
                Int32 top1, top2;
                w->mLastFrame->GetTop(&top1);
                w->mFrame->GetTop(&top2);
                winAnimator->mAnimDh = top1 - top2;

                //TODO (multidisplay): Accessibility supported only for the default display.
                if (mAccessibilityController != NULL && displayId == IDisplay::DEFAULT_DISPLAY) {
                    mAccessibilityController->OnSomeWindowResizedOrMovedLocked();
                }

                // try {
                w->mClient->Moved(left2, top2);
                // } catch (RemoteException e) {
                // }
            }

            //Slog.i(TAG, "Window " + this + " clearing mContentChanged - done placing");
            w->mContentChanged = FALSE;

            // Moved from updateWindowsAndWallpaperLocked().
            if (w->mHasSurface) {
                // Take care of the window being ready to display.
                Boolean committed =
                        winAnimator->CommitFinishDrawingLocked(currentTime);
                if (isDefaultDisplay && committed) {
                    Int32 type;
                    w->mAttrs->GetType(&type);
                    if (type == IWindowManagerLayoutParams::TYPE_DREAM) {
                        // HACK: When a dream is shown, it may at that
                        // point hide the lock screen.  So we need to
                        // redo the layout to let the phone window manager
                        // make this happen.
                        displayContent->mPendingLayoutChanges |=
                                IWindowManagerPolicy::FINISH_LAYOUT_REDO_LAYOUT;
                        if (DEBUG_LAYOUT_REPEATS) {
                            DebugLayoutRepeats(
                                    String("dream and commitFinishDrawingLocked true"),
                                    displayContent->mPendingLayoutChanges);
                        }
                    }
                    Int32 flags;
                    w->mAttrs->GetFlags(&flags);
                    if ((flags & IWindowManagerLayoutParams::FLAG_SHOW_WALLPAPER) != 0) {
                        // if (DEBUG_WALLPAPER_LIGHT) Slog.v(TAG,
                        //         "First draw done in potential wallpaper target " + w);
                        mInnerFields->mWallpaperMayChange = TRUE;
                        displayContent->mPendingLayoutChanges |=
                                IWindowManagerPolicy::FINISH_LAYOUT_REDO_WALLPAPER;
                        if (DEBUG_LAYOUT_REPEATS) {
                            DebugLayoutRepeats(
                                    String("wallpaper and commitFinishDrawingLocked true"),
                                    displayContent->mPendingLayoutChanges);
                        }
                    }
                }

                winAnimator->SetSurfaceBoundariesLocked(recoveringMemory);

                AutoPtr<AppWindowToken> atoken = w->mAppToken;
                if (DEBUG_STARTING_WINDOW && atoken != NULL
                        && w == atoken->mStartingWindow) {
                    Slogger::D(TAG, "updateWindows: starting %p isOnScreen=%d allDrawn=%d freezingScreen=%p",
                        w.Get(), w->IsOnScreen(), atoken->mAllDrawn, atoken->mAppAnimator->mFreezingScreen);
                }
                if (atoken != NULL
                        && (!atoken->mAllDrawn || atoken->mAppAnimator->mFreezingScreen)) {
                    if (atoken->mLastTransactionSequence != mTransactionSequence) {
                        atoken->mLastTransactionSequence = mTransactionSequence;
                        atoken->mNumInterestingWindows = atoken->mNumDrawnWindows = 0;
                        atoken->mStartingDisplayed = FALSE;
                    }
                    if ((w->IsOnScreen() || winAnimator->mAttrType == IWindowManagerLayoutParams::TYPE_BASE_APPLICATION)
                            && !w->mExiting && !w->mDestroying) {
                        if (DEBUG_VISIBILITY || DEBUG_ORIENTATION) {
                            Slogger::V(TAG, "Eval win %p: isDrawn=%d, isAnimating=%d"
                                    , w.Get(), w->IsDrawnLw(), winAnimator->IsAnimating());
                            if (!w->IsDrawnLw()) {
                                Slogger::V(TAG, "Not displayed: s=%p pv=%d mDrawState=%d ah=%d th=%d a=%d"
                                        , winAnimator->mSurfaceControl.Get(), w->mPolicyVisibility
                                        , winAnimator->mDrawState, w->mAttachedHidden
                                        , atoken->mHiddenRequested, winAnimator->mAnimating);
                            }
                        }
                        if (w != atoken->mStartingWindow) {
                            if (!atoken->mAppAnimator->mFreezingScreen || !w->mAppFreezing) {
                                atoken->mNumInterestingWindows++;
                                if (w->IsDrawnLw()) {
                                    atoken->mNumDrawnWindows++;
                                    if (DEBUG_VISIBILITY || DEBUG_ORIENTATION) {
                                        Slogger::V(TAG, "tokenMayBeDrawn: %p freezingScreen=%d mAppFreezing=%d"
                                                , atoken.Get(), atoken->mAppAnimator->mFreezingScreen
                                                , w->mAppFreezing);
                                    }
                                    updateAllDrawn = TRUE;
                                }
                            }
                        }
                        else if (w->IsDrawnLw()) {
                            atoken->mStartingDisplayed = TRUE;
                        }
                    }
                }
            }

            Boolean isDisplayed;
            if (isDefaultDisplay && someoneLosingFocus && (w == mCurrentFocus)
                    && (w->IsDisplayedLw(&isDisplayed), isDisplayed)) {
                focusDisplayed = TRUE;
            }

            UpdateResizingWindows(w);
        }

        mDisplayManagerInternal->SetDisplayProperties(displayId,
                mInnerFields->mDisplayHasContent, mInnerFields->mPreferredRefreshRate,
                TRUE /* inTraversal, must call performTraversalInTrans... below */);

        GetDisplayContentLocked(displayId)->StopDimmingIfNeeded();

        if (updateAllDrawn) {
            UpdateAllDrawnLocked(displayContent);
        }
    }

    if (focusDisplayed) {
        Boolean result;
        mH->SendEmptyMessage(H::REPORT_LOSING_FOCUS, &result);
    }

    // Give the display manager a chance to adjust properties
    // like display rotation if it needs to.
    mDisplayManagerInternal->PerformTraversalInTransactionFromWindowManager();
    // } catch (RuntimeException e) {
    //     Log.wtf(TAG, "Unhandled exception in Window Manager", e);
    // } finally {
    helper->CloseTransaction();
    if (SHOW_LIGHT_TRANSACTIONS) Slogger::I(TAG, "<<< CLOSE TRANSACTION performLayoutAndPlaceSurfaces");
    // }

    AutoPtr<WindowList> defaultWindows = defaultDisplay->GetWindowList();

    // If we are ready to perform an app transition, check through
    // all of the app tokens to be shown and see if they are ready
    // to go.
    if (mAppTransition->IsReady()) {
        defaultDisplay->mPendingLayoutChanges |= HandleAppTransitionReadyLocked(defaultWindows);
        if (DEBUG_LAYOUT_REPEATS) DebugLayoutRepeats(String("after handleAppTransitionReadyLocked"),
            defaultDisplay->mPendingLayoutChanges);
    }

    if (!mAnimator->mAnimating && mAppTransition->IsRunning()) {
        // We have finished the animation of an app transition.  To do
        // this, we have delayed a lot of operations like showing and
        // hiding apps, moving apps in Z-order, etc.  The app token list
        // reflects the correct Z-order, but the window list may now
        // be out of sync with it.  So here we will just rebuild the
        // entire app window list.  Fun!
        defaultDisplay->mPendingLayoutChanges |= HandleAnimatingStoppedAndTransitionLocked();
        if (DEBUG_LAYOUT_REPEATS) DebugLayoutRepeats(String("after handleAnimStopAndXitionLock"),
            defaultDisplay->mPendingLayoutChanges);
    }

    if (mInnerFields->mWallpaperForceHidingChanged && defaultDisplay->mPendingLayoutChanges == 0
            && !mAppTransition->IsReady()) {
        // At this point, there was a window with a wallpaper that
        // was force hiding other windows behind it, but now it
        // is going away.  This may be simple -- just animate
        // away the wallpaper and its window -- or it may be
        // hard -- the wallpaper now needs to be shown behind
        // something that was hidden.
        defaultDisplay->mPendingLayoutChanges |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_LAYOUT;
        if (DEBUG_LAYOUT_REPEATS) DebugLayoutRepeats(String("after animateAwayWallpaperLocked"),
            defaultDisplay->mPendingLayoutChanges);
    }
    mInnerFields->mWallpaperForceHidingChanged = FALSE;

    if (mInnerFields->mWallpaperMayChange) {
        if (DEBUG_WALLPAPER_LIGHT) Slogger::V(TAG, "Wallpaper may change!  Adjusting");
        defaultDisplay->mPendingLayoutChanges |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_WALLPAPER;
        if (DEBUG_LAYOUT_REPEATS) DebugLayoutRepeats(String("WallpaperMayChange"),
                defaultDisplay->mPendingLayoutChanges);
    }

    if (mFocusMayChange) {
        mFocusMayChange = FALSE;
        if (UpdateFocusedWindowLocked(UPDATE_FOCUS_PLACING_SURFACES,
                FALSE /*updateInputWindows*/)) {
            updateInputWindowsNeeded = TRUE;
            defaultDisplay->mPendingLayoutChanges |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_ANIM;
        }
    }

    if (NeedsLayout()) {
        defaultDisplay->mPendingLayoutChanges |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_LAYOUT;
        if (DEBUG_LAYOUT_REPEATS) DebugLayoutRepeats(String("mLayoutNeeded"),
                defaultDisplay->mPendingLayoutChanges);
    }

    List< AutoPtr<WindowState> >::ReverseIterator resizingWinRIt = mResizingWindows.RBegin();
    while (resizingWinRIt != mResizingWindows.REnd()) {
        AutoPtr<WindowState> win = *resizingWinRIt;
        if (win->mAppFreezing) {
            // Don't remove this window until rotation has completed.
            ++resizingWinRIt;
            continue;
        }
        win->ReportResized();
        resizingWinRIt = List< AutoPtr<WindowState> >::ReverseIterator(
                mResizingWindows.Erase(--(resizingWinRIt.GetBase())));
        // } catch (RemoteException e) {
        //     win.mOrientationChanging = false;
        // }
    }

    if (DEBUG_ORIENTATION && mDisplayFrozen) {
        Slogger::V(TAG, "With display frozen, orientationChangeComplete=%d", mInnerFields->mOrientationChangeComplete);
    }
    if (mInnerFields->mOrientationChangeComplete) {
        if (mWindowsFreezingScreen) {
            mWindowsFreezingScreen = FALSE;
            mLastFinishedFreezeSource = mInnerFields->mLastWindowFreezeSource;
            mH->RemoveMessages(H::WINDOW_FREEZE_TIMEOUT);
        }
        StopFreezingDisplayLocked();
    }

    // Destroy the surface of any windows that are no longer visible.
    Boolean wallpaperDestroyed = FALSE;
    if (!mDestroySurface.IsEmpty()) {
        List< AutoPtr<WindowState> >::ReverseIterator destroyWinRit;
        for (destroyWinRit = mDestroySurface.RBegin(); destroyWinRit != mDestroySurface.REnd(); ++destroyWinRit) {
            AutoPtr<WindowState> win = *destroyWinRit;
            win->mDestroying = FALSE;
            if (mInputMethodWindow == win) {
                mInputMethodWindow = NULL;
            }
            if (win == mWallpaperTarget) {
                wallpaperDestroyed = TRUE;
            }
            win->mWinAnimator->DestroySurfaceLocked();
        }
        mDestroySurface.Clear();
    }

    // Time to remove any exiting tokens?
    for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
        AutoPtr<IInterface> value;
        mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
        AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
        List<AutoPtr<WindowToken> > exitingTokens = displayContent->mExitingTokens;
        List<AutoPtr<WindowToken> >::ReverseIterator exitTokenRit = exitingTokens.RBegin();
        while (exitTokenRit != exitingTokens.REnd()) {
            AutoPtr<WindowToken> token = (*exitTokenRit);
            if (!token->mHasVisible) {
                exitTokenRit = List<AutoPtr<WindowToken> >::ReverseIterator(
                        exitingTokens.Erase((++exitTokenRit).GetBase()));
                if (token->mWindowType == IWindowManagerLayoutParams::TYPE_WALLPAPER) {
                    mWallpaperTokens.Remove(token);
                }
                continue;
            }
            ++exitTokenRit;
        }
    }

    // Time to remove any exiting applications?
    mStackIdToStack->GetSize(&stackSize);
    for (Int32 stackNdx = stackSize - 1; stackNdx >= 0; --stackNdx) {
        // Initialize state of exiting applications.
        AutoPtr<IInterface> value;
        mStackIdToStack->ValueAt(stackNdx, (IInterface**)&value);
        AutoPtr<TaskStack> taskStack = (TaskStack*)(IObject*)value.Get();
        AppTokenList exitingAppTokens = taskStack->mExitingAppTokens;
        AppTokenList::ReverseIterator appTokenRit = exitingAppTokens.RBegin();
        while (appTokenRit != exitingAppTokens.REnd()) {
            AutoPtr<AppWindowToken> token = *appTokenRit;
            if (!token->mHasVisible &&
                    (Find(mClosingApps.Begin(), mClosingApps.End(), token) == mClosingApps.End()) &&
                    !token->mDeferRemoval) {
                // Make sure there is no animation running on this token,
                // so any windows associated with it will be removed as
                // soon as their animations are complete
                token->mAppAnimator->ClearAnimation();
                token->mAppAnimator->mAnimating = FALSE;
                if (DEBUG_ADD_REMOVE || DEBUG_TOKEN_MOVEMENT)
                    Slogger::V(TAG, "performLayout: App token exiting now removed%p", token.Get());
                RemoveAppFromTaskLocked(token);
                appTokenRit = List< AutoPtr<AppWindowToken> >::ReverseIterator(
                        exitingAppTokens.Erase(--(appTokenRit.GetBase())));
                continue;
            }
            ++appTokenRit;
        }
    }

    if (!mAnimator->mAnimating &&
            mRelayoutWhileAnimating.Begin() != mRelayoutWhileAnimating.End()) {
        WindowList::ReverseIterator rit = mRelayoutWhileAnimating.RBegin();
        for (; rit != mRelayoutWhileAnimating.REnd(); ++rit) {
            // try {
            (*rit)->mClient->DoneAnimating();
            // } catch (RemoteException e) {
            // }
        }
        mRelayoutWhileAnimating.Clear();
    }

    if (wallpaperDestroyed) {
        defaultDisplay->mPendingLayoutChanges |= IWindowManagerPolicy::FINISH_LAYOUT_REDO_WALLPAPER;
        defaultDisplay->mLayoutNeeded = TRUE;
    }

    for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
        AutoPtr<IInterface> value;
        mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
        AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
        if (displayContent->mPendingLayoutChanges != 0) {
            displayContent->mLayoutNeeded = TRUE;
        }
    }

    // Finally update all input windows now that the window changes have stabilized.
    mInputMonitor->UpdateInputWindowsLw(TRUE /*force*/);

    SetHoldScreenLocked((CSession*)mInnerFields->mHoldScreen.Get());
    if (!mDisplayFrozen) {
        if (mInnerFields->mScreenBrightness < 0 || mInnerFields->mScreenBrightness > 1.0f) {
            mPowerManagerInternal->SetScreenBrightnessOverrideFromWindowManager(-1);
        }
        else {
            mPowerManagerInternal->SetScreenBrightnessOverrideFromWindowManager(
                    ToBrightnessOverride(mInnerFields->mScreenBrightness));
        }
        if (mInnerFields->mButtonBrightness < 0 || mInnerFields->mButtonBrightness > 1.0f) {
            mPowerManagerInternal->SetButtonBrightnessOverrideFromWindowManager(-1);
        }
        else {
            mPowerManagerInternal->SetButtonBrightnessOverrideFromWindowManager(
                    ToBrightnessOverride(mInnerFields->mButtonBrightness));
        }
        mPowerManagerInternal->SetUserActivityTimeoutOverrideFromWindowManager(
                mInnerFields->mUserActivityTimeout);
    }

    if (mTurnOnScreen) {
        if (DEBUG_VISIBILITY) Slogger::V(TAG, "Turning screen on after layout!");
        mPowerManager->WakeUp(SystemClock::GetUptimeMillis());
        mTurnOnScreen = FALSE;
    }

    if (mInnerFields->mUpdateRotation) {
        if (DEBUG_ORIENTATION) Slogger::D(TAG, "Performing post-rotate rotation");
        if (UpdateRotationUncheckedLocked(FALSE)) {
            Boolean result;
            mH->SendEmptyMessage(H::SEND_NEW_CONFIGURATION, &result);
        }
        else {
            mInnerFields->mUpdateRotation = FALSE;
        }
    }

    if (mWaitingForDrawnCallback != NULL ||
            (mInnerFields->mOrientationChangeComplete && !defaultDisplay->mLayoutNeeded &&
                    !mInnerFields->mUpdateRotation)) {
        CheckDrawnWindowsLocked();
    }

    Int32 N = mPendingRemove.GetSize();
    if (N > 0) {
        if (mPendingRemoveTmp->GetLength() < N) {
            mPendingRemoveTmp = ArrayOf<WindowState*>::Alloc(N + 10);
        }
        List< AutoPtr<WindowState> >::Iterator winIt = mPendingRemove.Begin();
        Int32 i = 0;
        for (; winIt != mPendingRemove.End(); ++winIt, ++i) {
            mPendingRemoveTmp->Set(i, *winIt);
        }
        mPendingRemove.Clear();
        List< AutoPtr<DisplayContent> > displayList;
        List< AutoPtr<DisplayContent> >::Iterator dcit;
        for (i = 0; i < N; i++) {
            AutoPtr<WindowState> w = (*mPendingRemoveTmp)[i];
            RemoveWindowInnerLocked(w->mSession, w);
            AutoPtr<DisplayContent> displayContent = w->GetDisplayContent();
            if (displayContent != NULL &&
                    (Find(displayList.Begin(), displayList.End(), displayContent) == displayList.End())) {
                displayList.PushBack(displayContent);
            }
        }

        for (dcit = displayList.Begin(); dcit != displayList.End(); ++dcit) {
            AssignLayersLocked((*dcit)->GetWindowList());
            (*dcit)->mLayoutNeeded = TRUE;
        }
    }

    // Remove all deferred displays stacks, tasks, and activities.
    Int32 contentsSize;
    mDisplayContents->GetSize(&contentsSize);
    for (Int32 displayNdx = contentsSize - 1; displayNdx >= 0; --displayNdx) {
        AutoPtr<IInterface> value;
        mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
        AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
        displayContent->CheckForDeferredActions();
    }

    if (updateInputWindowsNeeded) {
        mInputMonitor->UpdateInputWindowsLw(FALSE /*force*/);
    }
    SetFocusedStackFrame();

    // Check to see if we are now in a state where the screen should
    // be enabled, because the window obscured flags have changed.
    EnableScreenIfNeededLocked();

    ScheduleAnimationLocked();

    // if (DEBUG_WINDOW_TRACE) {
    //     Slog.e(TAG, "performLayoutAndPlaceSurfacesLockedInner exit: animating="
    //             + mAnimator.mAnimating);
    // }
}

Int32 CWindowManagerService::ToBrightnessOverride(
    /* [in] */ Float value)
{
    return (Int32)(value * IPowerManager::BRIGHTNESS_ON);
}

void CWindowManagerService::CheckDrawnWindowsLocked()
{
    if (mWaitingForDrawn.Begin() == mWaitingForDrawn.End() || mWaitingForDrawnCallback == NULL) {
        return;
    }
    WindowList::ReverseIterator rit = mWaitingForDrawn.RBegin();
    while (rit != mWaitingForDrawn.REnd()) {
        AutoPtr<WindowState> win = *rit;
        if (DEBUG_SCREEN_ON) {
            Boolean isVisible;
            win->IsVisibleLw(&isVisible);
            Slogger::I(TAG, "Waiting for drawn %p: removed=%d visible=%d mHasSurface=%d drawState=%d"
                , win.Get(), win->mRemoved, isVisible, win->mHasSurface, win->mWinAnimator->mDrawState);
        }
        Boolean hasDrawn;
        if (win->mRemoved || !win->mHasSurface) {
            // Window has been removed; no draw will now happen, so stop waiting.
            if (DEBUG_SCREEN_ON) Slogger::W(TAG, "Aborted waiting for drawn: %p", win.Get());
            rit = WindowList::ReverseIterator(mWaitingForDrawn.Erase(--(rit.GetBase())));
            continue;
        }
        else if (win->HasDrawnLw(&hasDrawn), hasDrawn) {
            // Window is now drawn (and shown).
            if (DEBUG_SCREEN_ON) Slogger::D(TAG, "Window drawn win=%p", win.Get());
            rit = WindowList::ReverseIterator(mWaitingForDrawn.Erase(--(rit.GetBase())));
            continue;
        }
        ++rit;
    }
    if (mWaitingForDrawn.Begin() == mWaitingForDrawn.End()) {
        if (DEBUG_SCREEN_ON) Slogger::D(TAG, "All windows drawn!");
        mH->RemoveMessages(H::WAITING_FOR_DRAWN_TIMEOUT);
        Boolean result;
        mH->SendEmptyMessage(H::ALL_WINDOWS_DRAWN, &result);
    }
}

void CWindowManagerService::SetHoldScreenLocked(
    /* [in] */ CSession* newHoldScreen)
{
    Boolean hold = newHoldScreen != NULL;

    if (hold && mHoldingScreenOn.Get() != newHoldScreen) {
        AutoPtr<IWorkSource> workSource;
        CWorkSource::New(newHoldScreen->mUid, (IWorkSource**)&workSource);
        mHoldingScreenWakeLock->SetWorkSource(workSource);
    }
    mHoldingScreenOn = newHoldScreen;

    Boolean state;
    mHoldingScreenWakeLock->IsHeld(&state);
    if (hold != state) {
        if (hold) {
            mHoldingScreenWakeLock->AcquireLock();
            mPolicy->KeepScreenOnStartedLw();
        }
        else {
            mPolicy->KeepScreenOnStoppedLw();
            mHoldingScreenWakeLock->ReleaseLock();
        }
    }
}

ECode CWindowManagerService::RequestTraversal()
{
    synchronized (mWindowMapLock) {
        RequestTraversalLocked();
    }
    return NOERROR;
}

void CWindowManagerService::RequestTraversalLocked()
{
    if (!mTraversalScheduled) {
        mTraversalScheduled = TRUE;
        Boolean result;
        mH->SendEmptyMessage(H::DO_TRAVERSAL, &result);
    }
}

void CWindowManagerService::ScheduleAnimationLocked()
{
    if (!mAnimationScheduled) {
        mAnimationScheduled = TRUE;
        mChoreographer->PostCallback(
                IChoreographer::CALLBACK_ANIMATION, mAnimator->mAnimationRunnable, NULL);
    }
}

Boolean CWindowManagerService::NeedsLayout()
{
    Int32 numDisplays;
    mDisplayContents->GetSize(&numDisplays);
    for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
        AutoPtr<IInterface> value;
        mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
        AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
        if (displayContent->mLayoutNeeded) {
            return TRUE;
        }
    }
    return FALSE;
}

Boolean CWindowManagerService::CopyAnimToLayoutParamsLocked()
{
    Boolean doRequest = FALSE;
    Int32 bulkUpdateParams = mAnimator->mBulkUpdateParams;
    if ((bulkUpdateParams & LayoutFields::SET_UPDATE_ROTATION) != 0) {
        mInnerFields->mUpdateRotation = TRUE;
        doRequest = TRUE;
    }
    if ((bulkUpdateParams & LayoutFields::SET_WALLPAPER_MAY_CHANGE) != 0) {
        mInnerFields->mWallpaperMayChange = TRUE;
        doRequest = TRUE;
    }
    if ((bulkUpdateParams & LayoutFields::SET_FORCE_HIDING_CHANGED) != 0) {
        mInnerFields->mWallpaperForceHidingChanged = TRUE;
        doRequest = TRUE;
    }
    if ((bulkUpdateParams & LayoutFields::SET_ORIENTATION_CHANGE_COMPLETE) == 0) {
        mInnerFields->mOrientationChangeComplete = FALSE;
    }
    else {
        mInnerFields->mOrientationChangeComplete = TRUE;
        mInnerFields->mLastWindowFreezeSource = mAnimator->mLastWindowFreezeSource;
        if (mWindowsFreezingScreen) {
            doRequest = TRUE;
        }
    }
    if ((bulkUpdateParams & LayoutFields::SET_TURN_ON_SCREEN) != 0) {
        mTurnOnScreen = TRUE;
    }
    if ((bulkUpdateParams & LayoutFields::SET_WALLPAPER_ACTION_PENDING) != 0) {
        mInnerFields->mWallpaperActionPending = TRUE;
    }

    return doRequest;
}

Int32 CWindowManagerService::AdjustAnimationBackground(
    /* [in] */ WindowStateAnimator* winAnimator)
{
    AutoPtr<WindowList> windows = winAnimator->mWin->GetWindowList();
    WindowList::ReverseIterator rit = windows->RBegin();
    for (; rit != windows->REnd(); ++rit) {
        AutoPtr<WindowState> testWin = *rit;
        if (testWin->mIsWallpaper && testWin->IsVisibleNow()) {
            return testWin->mWinAnimator->mAnimLayer;
        }
    }
    return winAnimator->mAnimLayer;
}

Boolean CWindowManagerService::ReclaimSomeSurfaceMemoryLocked(
    /* [in] */ WindowStateAnimator* winAnimator,
    /* [in] */ const String& operation,
    /* [in] */ Boolean secure)
{
    AutoPtr<ISurfaceControl> surface = winAnimator->mSurfaceControl;
    Boolean leakedSurface = FALSE;
    Boolean killedApps = FALSE;

    // EventLog.writeEvent(EventLogTags.WM_NO_SURFACE_MEMORY, winAnimator.mWin.toString(),
    //         winAnimator.mSession.mPid, operation);

    if (mForceRemoves == NULL) {
        mForceRemoves = new List< AutoPtr<WindowState> >();
    }

    Int64 callingIdentity = Binder::ClearCallingIdentity();
    // try {
        // There was some problem...   first, do a sanity check of the
        // window list to make sure we haven't left any dangling surfaces
        // around.

    Slogger::I(TAG, "Out of memory for surface!  Looking for leaks...");
    Int32 numDisplays;
    mDisplayContents->GetSize(&numDisplays);
    for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
        AutoPtr<IInterface> value;
        mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
        AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
        AutoPtr<WindowList> windows = displayContent->GetWindowList();
        WindowList::Iterator winIt = windows->Begin();
        for (; winIt != windows->End(); ++winIt) {
            AutoPtr<WindowState> ws = *winIt;
            AutoPtr<WindowStateAnimator> wsa = ws->mWinAnimator;
            if (wsa->mSurfaceControl != NULL) {
                Boolean contains;
                if (ISet::Probe(mSessions)->Contains((IWindowSession*)wsa->mSession.Get(), &contains), !contains) {
                    Slogger::W(TAG, "LEAKED SURFACE (session doesn't exist): %p surface=%p token=%p pid=%d uid=%d"
                            , ws.Get(), wsa->mSurfaceControl.Get(), ws->mToken.Get()
                            , ws->mSession->mPid, ws->mSession->mUid);
                    // if (SHOW_TRANSACTIONS) LogSurface(ws, String("LEAK DESTROY"), 0);
                    wsa->mSurfaceControl->Destroy();
                    wsa->mSurfaceShown = FALSE;
                    wsa->mSurfaceControl = NULL;
                    ws->mHasSurface = FALSE;
                    mForceRemoves->PushBack(ws);
                    leakedSurface = TRUE;
                }
                else if (ws->mAppToken != NULL && ws->mAppToken->mClientHidden) {
                    Slogger::W(TAG, "LEAKED SURFACE (app token hidden): %p surface=%p token=%p"
                            , ws.Get(), wsa->mSurfaceControl.Get(), ws->mAppToken);
                    // if (SHOW_TRANSACTIONS) LogSurface(ws, String("LEAK DESTROY"), 0);
                    wsa->mSurfaceControl->Destroy();
                    wsa->mSurfaceShown = FALSE;
                    wsa->mSurfaceControl = NULL;
                    ws->mHasSurface = FALSE;
                    leakedSurface = TRUE;
                }
            }
        }
    }

    if (!leakedSurface) {
        Slogger::W(TAG, "No leaked surfaces; killing applicatons!");
        AutoPtr<ISparseInt32Array> pidCandidates;
        CSparseInt32Array::New((ISparseInt32Array**)&pidCandidates);
        for (Int32 displayNdx = 0; displayNdx < numDisplays; ++displayNdx) {
            AutoPtr<IInterface> value;
            mDisplayContents->ValueAt(displayNdx, (IInterface**)&value);
            AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
            AutoPtr<WindowList> windows = displayContent->GetWindowList();
            WindowList::Iterator winIt = windows->Begin();
            for (; winIt != windows->End(); ++winIt) {
                AutoPtr<WindowState> ws = *winIt;
                if (Find(mForceRemoves->Begin(), mForceRemoves->End(), ws) != mForceRemoves->End()) {
                    continue;
                }
                AutoPtr<WindowStateAnimator> wsa = ws->mWinAnimator;
                if (wsa->mSurfaceControl != NULL) {
                    pidCandidates->Append(wsa->mSession->mPid, wsa->mSession->mPid);
                }
            }
            Int32 size;
            if (pidCandidates->GetSize(&size), size > 0) {
                AutoPtr<ArrayOf<Int32> > pids = ArrayOf<Int32>::Alloc(size);
                for (Int32 i = 0; i < pids->GetLength(); i++) {
                    Int32 value;
                    pidCandidates->KeyAt(i, &value);
                    (*pids)[i] = value;
                }
                // try {
                Boolean result;
                if (mActivityManager->KillPids(pids, String("Free memory"), secure, &result), result) {
                    killedApps = TRUE;
                }
                // } catch (RemoteException e) {
                // }
            }
        }
    }

    if (leakedSurface || killedApps) {
        // We managed to reclaim some memory, so get rid of the trouble
        // surface and ask the app to request another one.
        Slogger::W(TAG, "Looks like we have reclaimed some memory, clearing surface for retry.");
        if (surface != NULL) {
            // if (SHOW_TRANSACTIONS || SHOW_SURFACE_ALLOC)
            //     LogSurface(winAnimator->mWin, String("RECOVER DESTROY"), 0);
            surface->Destroy();
            winAnimator->mSurfaceShown = FALSE;
            winAnimator->mSurfaceControl = NULL;
            winAnimator->mWin->mHasSurface = FALSE;
            ScheduleRemoveStartingWindowLocked(winAnimator->mWin->mAppToken);
        }

        // try {
        winAnimator->mWin->mClient->DispatchGetNewSurface();
        // } catch (RemoteException e) {
        // }
    }
    // } finally {
    //     Binder::RestoreCallingIdentity(callingIdentity);
    // }
    Binder::RestoreCallingIdentity(callingIdentity);

    return leakedSurface || killedApps;
}

Boolean CWindowManagerService::UpdateFocusedWindowLocked(
    /* [in] */ Int32 mode,
    /* [in] */ Boolean updateInputWindows)
{
    AutoPtr<WindowState> newFocus = ComputeFocusedWindowLocked();
    if (mCurrentFocus != newFocus) {
        // AutoPtr<ITrace> trace;
        // CTrace::AcquireSingleton((ITrace**)&trace);
        // trace->TraceBegin(ITrace::TRACE_TAG_WINDOW_MANAGER, String("wmUpdateFocus"));
        // This check makes sure that we don't already have the focus
        // change message pending.
        Boolean result;
        mH->RemoveMessages(H::REPORT_FOCUS_CHANGE);
        mH->SendEmptyMessage(H::REPORT_FOCUS_CHANGE, &result);
        // TODO(multidisplay): Focused windows on default display only.
        AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();
        Boolean imWindowChanged = MoveInputMethodWindowsIfNeededLocked(
                mode != UPDATE_FOCUS_WILL_ASSIGN_LAYERS && mode != UPDATE_FOCUS_WILL_PLACE_SURFACES);
        if (imWindowChanged) {
            displayContent->mLayoutNeeded = TRUE;
            newFocus = ComputeFocusedWindowLocked();
        }

        if (DEBUG_FOCUS_LIGHT || localLOGV) {
            Slogger::V(TAG, "Changing focus from %p to %p  Callers="
                    , mCurrentFocus.Get(), newFocus.Get()/* + "" + Debug.getCallers(4)*/);
        }
        AutoPtr<WindowState> oldFocus = mCurrentFocus;
        mCurrentFocus = newFocus;
        mLosingFocus.Remove(newFocus);

        // TODO(multidisplay): Accessibilty supported only of default desiplay.
        if (mAccessibilityController != NULL
                && displayContent->GetDisplayId() == IDisplay::DEFAULT_DISPLAY) {
            mAccessibilityController->OnWindowFocusChangedLocked();
        }

        Int32 focusChanged;
        mPolicy->FocusChangedLw(oldFocus, newFocus, &focusChanged);

        if (imWindowChanged && oldFocus != mInputMethodWindow) {
            // Focus of the input method window changed. Perform layout if needed.
            if (mode == UPDATE_FOCUS_PLACING_SURFACES) {
                PerformLayoutLockedInner(displayContent, TRUE /*initial*/, updateInputWindows);
                focusChanged &= ~IWindowManagerPolicy::FINISH_LAYOUT_REDO_LAYOUT;
            }
            else if (mode == UPDATE_FOCUS_WILL_PLACE_SURFACES) {
                // Client will do the layout, but we need to assign layers
                // for handleNewWindowLocked() below.
                AssignLayersLocked(displayContent->GetWindowList());
            }
        }

        if ((focusChanged & IWindowManagerPolicy::FINISH_LAYOUT_REDO_LAYOUT) != 0) {
            // The change in focus caused us to need to do a layout.  Okay.
            displayContent->mLayoutNeeded = TRUE;
            if (mode == UPDATE_FOCUS_PLACING_SURFACES) {
                PerformLayoutLockedInner(displayContent, TRUE /*initial*/, updateInputWindows);
            }
        }

        if (mode != UPDATE_FOCUS_WILL_ASSIGN_LAYERS) {
            // If we defer assigning layers, then the caller is responsible for
            // doing this part.
            mInputMonitor->SetInputFocusLw(mCurrentFocus, updateInputWindows);
        }

        // tracse->TraceEnd(ITrace::TRACE_TAG_WINDOW_MANAGER);
        return TRUE;
    }
    return FALSE;
}

AutoPtr<WindowState> CWindowManagerService::ComputeFocusedWindowLocked()
{
    if (mAnimator->mUniverseBackground != NULL
            && mAnimator->mUniverseBackground->mWin->CanReceiveKeys()) {
        return mAnimator->mUniverseBackground->mWin;
    }

    Int32 displayCount;
    mDisplayContents->GetSize(&displayCount);
    for (Int32 i = 0; i < displayCount; i++) {
        AutoPtr<IInterface> value;
        mDisplayContents->ValueAt(i, (IInterface**)&value);
        AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
        AutoPtr<WindowState> win = FindFocusedWindowLocked(displayContent);
        if (win != NULL) {
            return win;
        }
    }
    return NULL;
}

AutoPtr<WindowState> CWindowManagerService::FindFocusedWindowLocked(
    /* [in] */ DisplayContent* displayContent)
{
    AutoPtr<WindowList> windows = displayContent->GetWindowList();
    WindowList::ReverseIterator rit;
    for (rit = windows->RBegin(); rit != windows->REnd(); ++rit) {
        AutoPtr<WindowState> win = *rit;

        // if (localLOGV || DEBUG_FOCUS) Slog.v(
        //     TAG, "Looking for focus: " + i
        //     + " = " + win
        //     + ", flags=" + win.mAttrs.flags
        //     + ", canReceive=" + win.canReceiveKeys());

        AutoPtr<AppWindowToken> wtoken = win->mAppToken;

        // If this window's application has been removed, just skip it.
        if (wtoken != NULL && (wtoken->mRemoved || wtoken->mSendingToBottom)) {
            if (DEBUG_FOCUS)
                Slogger::V(TAG, "Skipping %p because %s", wtoken.Get(),  (wtoken->mRemoved ? "removed" : "sendingToBottom"));
            continue;
        }

        if (!win->CanReceiveKeys()) {
            continue;
        }

        // Descend through all of the app tokens and find the first that either matches
        // win.mAppToken (return win) or mFocusedApp (return null).
        Int32 type;
        if (wtoken != NULL && (win->mAttrs->GetType(&type), type != IWindowManagerLayoutParams::TYPE_APPLICATION_STARTING) &&
                mFocusedApp != NULL) {
            AutoPtr<List<AutoPtr<Task> > > tasks = displayContent->GetTasks();
            List<AutoPtr<Task> >::ReverseIterator taskRit = tasks->RBegin();
            for (; taskRit != tasks->REnd(); ++taskRit) {
                AppTokenList tokens = (*taskRit)->mAppTokens;
                AppTokenList::ReverseIterator tokenRit = tokens.RBegin();
                for (; tokenRit != tokens.REnd(); ++tokenRit) {
                    AutoPtr<AppWindowToken> token = *tokenRit;
                    if (wtoken == token) {
                        break;
                    }
                    if (mFocusedApp == token) {
                        // Whoops, we are below the focused app...  no focus for you!
                        if (localLOGV || DEBUG_FOCUS_LIGHT)
                            Slogger::V(TAG, "findFocusedWindow: Reached focused app=%p", mFocusedApp.Get());
                        return NULL;
                    }
                }
                if (tokenRit != tokens.REnd()) {
                    // Early exit from loop, must have found the matching token.
                    break;
                }
            }
        }

        // if (DEBUG_FOCUS_LIGHT) Slogger::V(TAG, "findFocusedWindow: Found new focus @ %d = %p", i, win.Get());
        return win;
    }

    if (DEBUG_FOCUS_LIGHT) Slogger::V(TAG, "findFocusedWindow: No focusable windows.");
    return NULL;
}

void CWindowManagerService::StartFreezingDisplayLocked(
    /* [in] */ Boolean inTransaction,
    /* [in] */ Int32 exitAnim,
    /* [in] */ Int32 enterAnim)
{
    if (mDisplayFrozen) {
        return;
    }

    Boolean isON;
    if (!mDisplayReady || (mPolicy->IsScreenOn(&isON), !isON)) {
        // No need to freeze the screen before the system is ready or if
        // the screen is off.
        return;
    }

    mScreenFrozenLock->AcquireLock();

    mDisplayFrozen = TRUE;
    mDisplayFreezeTime = SystemClock::GetElapsedRealtime();
    mLastFinishedFreezeSource = NULL;

    mInputMonitor->FreezeInputDispatchingLw();

    // Clear the last input window -- that is just used for
    // clean transitions between IMEs, and if we are freezing
    // the screen then the whole world is changing behind the scenes.
    mPolicy->SetLastInputMethodWindowLw(NULL, NULL);

    if (mAppTransition->IsTransitionSet()) {
        mAppTransition->Freeze();
    }

    // if (PROFILE_ORIENTATION) {
    //     File file = new File("/data/system/frozen");
    //     Debug.startMethodTracing(file.toString(), 8 * 1024 * 1024);
    // }

    if (CUSTOM_SCREEN_ROTATION) {
        mExitAnimId = exitAnim;
        mEnterAnimId = enterAnim;
        AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();
        Int32 displayId = displayContent->GetDisplayId();
        AutoPtr<ScreenRotationAnimation> screenRotationAnimation =
                mAnimator->GetScreenRotationAnimationLocked(displayId);
        if (screenRotationAnimation != NULL) {
            screenRotationAnimation->Kill();
        }

        // Check whether the current screen contains any secure content.
        Boolean isSecure = FALSE;
        AutoPtr<WindowList> windows = GetDefaultWindowListLocked();
        WindowList::Iterator winIt = windows->Begin();
        for (; winIt != windows->End(); ++winIt) {
            AutoPtr<WindowState> ws = *winIt;
            Int32 flags;
            if (ws->IsOnScreen() && (ws->mAttrs->GetFlags(&flags), (flags & IWindowManagerLayoutParams::FLAG_SECURE) != 0)) {
                isSecure = TRUE;
                break;
            }
        }

        // TODO(multidisplay): rotation on main screen only.
        displayContent->UpdateDisplayInfo();
        Boolean isForced;
        mPolicy->IsDefaultOrientationForced(&isForced);
        screenRotationAnimation = new ScreenRotationAnimation(mContext, displayContent,
                mFxSession, inTransaction, isForced, isSecure);
        mAnimator->SetScreenRotationAnimationLocked(displayId, screenRotationAnimation);
    }
}

void CWindowManagerService::StopFreezingDisplayLocked()
{
    if (!mDisplayFrozen) {
        return;
    }

    if (mWaitingForConfig || mAppsFreezingScreen > 0 || mWindowsFreezingScreen
            || mClientFreezingScreen) {
        // if (DEBUG_ORIENTATION) Slog.d(TAG,
        //     "stopFreezingDisplayLocked: Returning mWaitingForConfig=" + mWaitingForConfig
        //     + ", mAppsFreezingScreen=" + mAppsFreezingScreen
        //     + ", mWindowsFreezingScreen=" + mWindowsFreezingScreen
        //     + ", mClientFreezingScreen=" + mClientFreezingScreen);
        return;
    }

    mDisplayFrozen = FALSE;
    mLastDisplayFreezeDuration = (Int32)(SystemClock::GetElapsedRealtime() - mDisplayFreezeTime);
    StringBuilder sb(128);
    sb.Append("Screen frozen for ");
    TimeUtils::FormatDuration(mLastDisplayFreezeDuration, sb);
    if (mLastFinishedFreezeSource != NULL) {
        sb.Append(" due to ");
        sb.Append(mLastFinishedFreezeSource);
    }
    Slogger::I(TAG, "%s", sb.ToString().string());
    mH->RemoveMessages(H::APP_FREEZE_TIMEOUT);
    mH->RemoveMessages(H::CLIENT_FREEZE_TIMEOUT);

    // if (PROFILE_ORIENTATION) {
    //     Debug.stopMethodTracing();
    // }

    Boolean updateRotation = FALSE;

    AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();
    Int32 displayId = displayContent->GetDisplayId();
    AutoPtr<ScreenRotationAnimation> screenRotationAnimation =
            mAnimator->GetScreenRotationAnimationLocked(displayId);
    if (CUSTOM_SCREEN_ROTATION && screenRotationAnimation != NULL
            && screenRotationAnimation->HasScreenshot()) {
        // if (DEBUG_ORIENTATION) Slog.i(TAG, "**** Dismissing screen rotation animation");
        // TODO(multidisplay): rotation on main screen only.
        AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
        // Get rotation animation again, with new top window
        Boolean isDimming = displayContent->IsDimming();
        Boolean result;
        if (mPolicy->ValidateRotationAnimationLw(mExitAnimId, mEnterAnimId, isDimming, &result), !result) {
            mExitAnimId = mEnterAnimId = 0;
        }
        Int32 width, height;
        displayInfo->GetLogicalWidth(&width);
        displayInfo->GetLogicalHeight(&height);
        if (screenRotationAnimation->Dismiss(mFxSession, MAX_ANIMATION_DURATION,
                GetTransitionAnimationScaleLocked(), width, height, mExitAnimId, mEnterAnimId)) {
            ScheduleAnimationLocked();
        }
        else {
            screenRotationAnimation->Kill();
            screenRotationAnimation = NULL;
            mAnimator->SetScreenRotationAnimationLocked(displayId, screenRotationAnimation);
            updateRotation = TRUE;
        }
    }
    else {
        if (screenRotationAnimation != NULL) {
            screenRotationAnimation->Kill();
            screenRotationAnimation = NULL;
            mAnimator->SetScreenRotationAnimationLocked(displayId, screenRotationAnimation);
        }
        updateRotation = TRUE;
    }

    mInputMonitor->ThawInputDispatchingLw();

    // While the display is frozen we don't re-compute the orientation
    // to avoid inconsistent states.  However, something interesting
    // could have actually changed during that time so re-evaluate it
    // now to catch that.
    Boolean configChanged = UpdateOrientationFromAppTokensLocked(FALSE);

    // A little kludge: a lot could have happened while the
    // display was frozen, so now that we are coming back we
    // do a gc so that any remote references the system
    // processes holds on others can be released if they are
    // no longer needed.
    Boolean result;
    mH->RemoveMessages(H::FORCE_GC);
    mH->SendEmptyMessageDelayed(H::FORCE_GC, 2000, &result);

    mScreenFrozenLock->ReleaseLock();

    if (updateRotation) {
        // if (DEBUG_ORIENTATION) Slog.d(TAG, "Performing post-rotate rotation");
        configChanged |= UpdateRotationUncheckedLocked(FALSE);
    }

    if (configChanged) {
        Boolean result;
        mH->SendEmptyMessage(H::SEND_NEW_CONFIGURATION, &result);
    }
}

Int32 CWindowManagerService::GetPropertyInt(
    /* [in] */ ArrayOf<String>* tokens,
    /* [in] */ Int32 index,
    /* [in] */ Int32 defUnits,
    /* [in] */ Int32 defDps,
    /* [in] */ IDisplayMetrics* dm)
{
    if (index < tokens->GetLength()) {
        String str = (*tokens)[index];
        if (!str.IsNullOrEmpty()) {
           // try {
            Int32 val = StringUtils::ParseInt32(str);
            return val;
           // } catch (Exception e) {
           // }
        }
    }
    if (defUnits == ITypedValue::COMPLEX_UNIT_PX) {
        return defDps;
    }
    AutoPtr<ITypedValueHelper> helper;
    CTypedValueHelper::AcquireSingleton((ITypedValueHelper**)&helper);
    Float val;
    helper->ApplyDimension(defUnits, defDps, dm, &val);
    return (Int32)val;
}

void CWindowManagerService::CreateWatermarkInTransaction()
{
    if (mWatermark != NULL) {
        return;
    }

    AutoPtr<IFile> file;
    CFile::New(String("/system/etc/setup.conf"), (IFile**)&file);
    AutoPtr<IFileInputStream> in;
    AutoPtr<IDataInputStream> ind;
    String line;
    // try {
    if (FAILED(CFileInputStream::New(file, (IFileInputStream**)&in))) {
        goto fail;
    }
    if (FAILED(CDataInputStream::New((IInputStream*)in.Get(), (IDataInputStream**)&ind))) {
        goto fail;
    }
    if (FAILED(IDataInput::Probe(ind.Get())->ReadLine(&line))) {
        goto fail;
    }
    if (!line.IsNull()) {
        AutoPtr< ArrayOf<String> > toks;
        StringUtils::Split(line, String("%"), (ArrayOf<String>**)&toks);
        if (toks != NULL && toks->GetLength() > 0) {
            AutoPtr<IDisplay> d = GetDefaultDisplayContentLocked()->GetDisplay();
            mWatermark = new Watermark(d, mRealDisplayMetrics, mFxSession, toks);
        }
    }
    // } catch (FileNotFoundException e) {
    // } catch (IOException e) {
    // } finally {
    //     if (in != null) {
    //         try {
    //             in.close();
    //         } catch (IOException e) {
    //         }
    //     }
    // }
fail:
    if (ind != NULL) {
        // try {
        ICloseable::Probe(ind.Get())->Close();
        // } catch (IOException e) {
        // }
    }
    else if (in != NULL) {
        // try {
        ICloseable::Probe(in.Get())->Close();
        // } catch (IOException e) {
        // }
    }
}

ECode CWindowManagerService::StatusBarVisibilityChanged(
    /* [in] */ Int32 visibility)
{
    Int32 perm;
    FAIL_RETURN(mContext->CheckCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::STATUS_BAR,
            &perm));
    if (perm != IPackageManager::PERMISSION_GRANTED) {
        Slogger::E(TAG, "Caller does not hold permission android.permission.STATUS_BAR");
        return E_SECURITY_EXCEPTION;
        // throw new SecurityException("Caller does not hold permission "
        //         + Elastos::Droid::Manifest::permission::STATUS_BAR);
    }

    synchronized (mWindowMapLock) {
        mLastStatusBarVisibility = visibility;
        mPolicy->AdjustSystemUiVisibilityLw(visibility, &visibility);
        UpdateStatusBarVisibilityLocked(visibility);
    }

    return NOERROR;
}

void CWindowManagerService::UpdateStatusBarVisibilityLocked(
    /* [in] */ Int32 visibility)
{
    mInputManager->SetSystemUiVisibility(visibility);
    AutoPtr<WindowList> windows = GetDefaultWindowListLocked();
    WindowList::Iterator it = windows->Begin();
    for (; it != windows->End(); ++it) {
        AutoPtr<WindowState> ws = *it;
        // try {
        Int32 curValue = ws->mSystemUiVisibility;
        Int32 diff = curValue ^ visibility;
        // We are only interested in differences of one of the
        // clearable flags...
        diff &= IView::SYSTEM_UI_CLEARABLE_FLAGS;
        // ...if it has actually been cleared.
        diff &= ~visibility;
        Int32 newValue = (curValue & ~diff) | (visibility & diff);
        if (newValue != curValue) {
            ws->mSeq++;
            ws->mSystemUiVisibility = newValue;
        }
        Boolean hasSystemUiListeners;
        ws->mAttrs->GetHasSystemUiListeners(&hasSystemUiListeners);
        if (newValue != curValue || hasSystemUiListeners) {
            ws->mClient->DispatchSystemUiVisibilityChanged(ws->mSeq,
                    visibility, newValue, diff);
        }
        // } catch (RemoteException e) {
        //     // so sorry
        // }
    }
}

ECode CWindowManagerService::ReevaluateStatusBarVisibility()
{
    synchronized (mWindowMapLock) {
        Int32 visibility;
        mPolicy->AdjustSystemUiVisibilityLw(mLastStatusBarVisibility, &visibility);
        UpdateStatusBarVisibilityLocked(visibility);
        PerformLayoutAndPlaceSurfacesLocked();
    }

    return NOERROR;
}

ECode CWindowManagerService::AddFakeWindow(
    /* [in] */ ILooper* looper,
    /* [in] */ IInputEventReceiverFactory* inputEventReceiverFactory,
    /* [in] */ const String& name,
    /* [in] */ Int32 windowType,
    /* [in] */ Int32 layoutParamsFlags,
    /* [in] */ Int32 layoutParamsPrivateFlags,
    /* [in] */ Boolean canReceiveKeys,
    /* [in] */ Boolean hasFocus,
    /* [in] */ Boolean touchFullscreen,
    /* [out] */ IFakeWindow** fakeWindow)
{
    VALIDATE_NOT_NULL(fakeWindow)

    synchronized (mWindowMapLock) {
        AutoPtr<FakeWindowImpl> fw = new FakeWindowImpl(this, looper, inputEventReceiverFactory,
                name, windowType, layoutParamsFlags, layoutParamsPrivateFlags,
                canReceiveKeys, hasFocus, touchFullscreen);
        List< AutoPtr<FakeWindowImpl> >::Iterator it = mFakeWindows.Begin();
        while (it != mFakeWindows.End()) {
            if ((*it)->mWindowLayer <= fw->mWindowLayer) {
                break;
            }
            it++;
        }
        mFakeWindows.Insert(it, fw);
        mInputMonitor->UpdateInputWindowsLw(TRUE);
        *fakeWindow = (IFakeWindow*)fw;
        REFCOUNT_ADD(*fakeWindow)
    }

    return NOERROR;
}

Boolean CWindowManagerService::RemoveFakeWindowLocked(
    /* [in] */ IFakeWindow* window)
{
    synchronized (mWindowMapLock) {
        List< AutoPtr<FakeWindowImpl> >::Iterator it = mFakeWindows.Begin();
        for (; it != mFakeWindows.End(); ++it) {
            if ((*it).Get() == window) {
                mFakeWindows.Erase(it);
                mInputMonitor->UpdateInputWindowsLw(TRUE);
                return TRUE;
            }
        }
    }
    return FALSE;
}

void CWindowManagerService::SaveLastInputMethodWindowForTransition()
{
    synchronized (mWindowMapLock) {
        // TODO(multidisplay): Pass in the displayID.
        AutoPtr<DisplayContent> displayContent = GetDefaultDisplayContentLocked();
        if (mInputMethodWindow != NULL) {
            mPolicy->SetLastInputMethodWindowLw(mInputMethodWindow, mInputMethodTarget);
        }
    }
}

Int32 CWindowManagerService::GetInputMethodWindowVisibleHeight()
{
    AutoLock lock(mWindowMapLock);
    Int32 height;
    mPolicy->GetInputMethodWindowVisibleHeightLw(&height);
    return height;
}

ECode CWindowManagerService::HasNavigationBar(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    return mPolicy->HasNavigationBar(result);
}

ECode CWindowManagerService::LockNow(
    /* [in] */ IBundle* options)
{
    return mPolicy->LockNow(options);
}

void CWindowManagerService::ShowRecentApps()
{
    mPolicy->ShowRecentApps();
}

ECode CWindowManagerService::IsSafeModeEnabled(
    /* [out] */ Boolean* isSafe)
{
    VALIDATE_NOT_NULL(isSafe)
    *isSafe = mSafeMode;
    return NOERROR;
}

ECode CWindowManagerService::ClearWindowContentFrameStats(
    /* [in] */ IBinder* token,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    *result = FALSE;

    if (!CheckCallingPermission(Elastos::Droid::Manifest::permission::FRAME_STATS,
            String("clearWindowContentFrameStats()"))) {
        Slogger::E(TAG, "Requires FRAME_STATS permission");
        return E_SECURITY_EXCEPTION;
    }
    AutoLock lock(mWindowMapLock);
    HashMap<AutoPtr<IBinder>, AutoPtr<WindowState> >::Iterator it = mWindowMap.Find(token);
    AutoPtr<WindowState> windowState;
    if (it != mWindowMap.End()) {
        windowState = it->mSecond;
    }
    if (windowState == NULL) {
        return NOERROR;
    }
    AutoPtr<ISurfaceControl> surfaceControl = windowState->mWinAnimator->mSurfaceControl;
    if (surfaceControl == NULL) {
        return NOERROR;
    }
    return surfaceControl->ClearContentFrameStats(result);
}

ECode CWindowManagerService::GetWindowContentFrameStats(
    /* [in] */ IBinder* token,
    /* [out] */ IWindowContentFrameStats** _stats)
{
    VALIDATE_NOT_NULL(_stats)
    *_stats = NULL;

    if (!CheckCallingPermission(Elastos::Droid::Manifest::permission::FRAME_STATS,
            String("getWindowContentFrameStats()"))) {
        Slogger::E(TAG, "Requires FRAME_STATS permission");
        return E_SECURITY_EXCEPTION;
    }
    synchronized (mWindowMapLock) {
        HashMap<AutoPtr<IBinder>, AutoPtr<WindowState> >::Iterator it = mWindowMap.Find(token);
        AutoPtr<WindowState> windowState;
        if (it != mWindowMap.End()) {
            windowState = it->mSecond;
        }
        if (windowState == NULL) {
            return NOERROR;
        }
        AutoPtr<ISurfaceControl> surfaceControl = windowState->mWinAnimator->mSurfaceControl;
        if (surfaceControl == NULL) {
            return NOERROR;
        }
        if (mTempWindowRenderStats == NULL) {
            CWindowContentFrameStats::New((IWindowContentFrameStats**)&mTempWindowRenderStats);
        }
        AutoPtr<IWindowContentFrameStats> stats = mTempWindowRenderStats;
        Boolean result;
        if (surfaceControl->GetContentFrameStats(stats, &result), !result) {
            return NOERROR;
        }
        *_stats = stats;
        REFCOUNT_ADD(*_stats)
    }

    return NOERROR;
}

void CWindowManagerService::SaveANRStateLocked(
    /* [in] */ AppWindowToken* appWindowToken,
    /* [in] */ WindowState* windowState,
    /* [in] */ const String& reason)
{
    AutoPtr<IStringWriter> sw;
    CStringWriter::New((IStringWriter**)&sw);
    AutoPtr<IFastPrintWriter> fpw;
    CFastPrintWriter::New(IWriter::Probe(sw), FALSE, 1024, (IFastPrintWriter**)&fpw);
    AutoPtr<IPrintWriter> pw = IPrintWriter::Probe(pw);
    AutoPtr<IDate> date;
    CDate::New((IDate**)&date);
    AutoPtr<IDateFormatHelper> dfh;
    CDateFormatHelper::AcquireSingleton((IDateFormatHelper **)&dfh);
    AutoPtr<IDateFormat> format;
    dfh->GetInstance((IDateFormat**)&format);
    String dStr;
    format->Format(date, &dStr);
    pw->Print(String("  ANR time: ") + dStr);
    if (appWindowToken != NULL) {
        pw->Println(String("  Application at fault: ") + appWindowToken->mStringName);
    }
    if (windowState != NULL) {
        AutoPtr<ICharSequence> title;
        windowState->mAttrs->GetTitle((ICharSequence**)&title);
        String tcs;
        title->ToString(&tcs);
        pw->Println(String("  Window at fault: ") + tcs);
    }
    if (!reason.IsNull()) {
        pw->Println(String("  Reason: ") + reason);
    }
    pw->Println();
    // DumpWindowsNoHeaderLocked(pw, TRUE, NULL);
    ICloseable::Probe(pw)->Close();
    // mLastANRState = sw->ToString();
}

ECode CWindowManagerService::Monitor()
{
    synchronized (mWindowMapLock) { }
    return NOERROR;
}

void CWindowManagerService::DebugLayoutRepeats(
    /* [in] */ const String& msg,
    /* [in] */ Int32 pendingLayoutChanges)
{
    if (mLayoutRepeatCount >= LAYOUT_REPEAT_THRESHOLD) {
        Slogger::V(TAG, "Layouts looping: %s, mPendingLayoutChanges = 0x%x", msg.string(),
                pendingLayoutChanges);
    }
}

AutoPtr<DisplayContent> CWindowManagerService::NewDisplayContentLocked(
    /* [in] */ IDisplay* display)
{
    AutoPtr<DisplayContent> displayContent = new DisplayContent(display, this);
    Int32 displayId;
    display->GetDisplayId(&displayId);
    if (DEBUG_DISPLAY) Slogger::V(TAG, "Adding display=%p", display);
    mDisplayContents->Put(displayId, (IObject*)displayContent.Get());

    AutoPtr<IDisplayInfo> displayInfo = displayContent->GetDisplayInfo();
    AutoPtr<IRect> rect;
    CRect::New((IRect**)&rect);
    String name;
    displayInfo->GetName(&name);
    mDisplaySettings->GetOverscanLocked(name, rect);
    AutoLock lock(displayContent->mDisplaySizeLock);
    Int32 left, top, right, bottom;
    rect->GetLeft(&left);
    rect->GetTop(&top);
    rect->GetRight(&right);
    rect->GetBottom(&bottom);
    displayInfo->SetOverscanLeft(left);
    displayInfo->SetOverscanTop(top);
    displayInfo->SetOverscanRight(right);
    displayInfo->SetOverscanBottom(bottom);
    mDisplayManagerInternal->SetDisplayInfoOverrideFromWindowManager(displayId, displayInfo);
    ConfigureDisplayPolicyLocked(displayContent);

    // TODO: Create an input channel for each display with touch capability.
    if (displayId == IDisplay::DEFAULT_DISPLAY) {
        displayContent->mTapDetector = new StackTapPointerEventListener(this, displayContent);
        RegisterPointerEventListener(displayContent->mTapDetector);
    }

    return displayContent;
}

void CWindowManagerService::CreateDisplayContentLocked(
    /* [in] */ IDisplay* display)
{
    // if (display == NULL) {
    //     throw new IllegalArgumentException("getDisplayContent: display must not be null");
    // }
    assert(display != NULL);
    Int32 id;
    display->GetDisplayId(&id);
    GetDisplayContentLocked(id);
}

AutoPtr<DisplayContent> CWindowManagerService::GetDisplayContentLocked(
    /* [in] */ Int32 displayId)
{
    AutoPtr<IInterface> value;
    mDisplayContents->Get(displayId, (IInterface**)&value);
    AutoPtr<DisplayContent> displayContent = (DisplayContent*)(IObject*)value.Get();
    if (displayContent == NULL) {
        AutoPtr<IDisplay> display;
        mDisplayManager->GetDisplay(displayId, (IDisplay**)&display);
        if (display != NULL) {
            displayContent = NewDisplayContentLocked(display);
        }
    }
    return displayContent;
}

AutoPtr<DisplayContent> CWindowManagerService::GetDefaultDisplayContentLocked()
{
    return GetDisplayContentLocked(IDisplay::DEFAULT_DISPLAY);
}

AutoPtr<List<AutoPtr<WindowState> > > CWindowManagerService::GetDefaultWindowListLocked()
{
    return GetDefaultDisplayContentLocked()->GetWindowList();
}

AutoPtr<IDisplayInfo> CWindowManagerService::GetDefaultDisplayInfoLocked()
{
    return GetDefaultDisplayContentLocked()->GetDisplayInfo();
}

AutoPtr<List<AutoPtr<WindowState> > > CWindowManagerService::GetWindowListLocked(
    /* [in] */ IDisplay* display)
{
    Int32 id;
    display->GetDisplayId(&id);
    return GetWindowListLocked(id);
}

AutoPtr<List<AutoPtr<WindowState> > > CWindowManagerService::GetWindowListLocked(
    /* [in] */ Int32 displayId)
{
    AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
    return displayContent != NULL ? displayContent->GetWindowList() : NULL;
}

ECode CWindowManagerService::OnDisplayAdded(
    /* [in] */ Int32 displayId)
{
    AutoPtr<IMessage> msg;
    mH->ObtainMessage(H::DO_DISPLAY_ADDED, displayId, 0, (IMessage**)&msg);
    Boolean result;
    return mH->SendMessage(msg, &result);
}

void CWindowManagerService::HandleDisplayAdded(
    /* [in] */ Int32 displayId)
{
    synchronized (mWindowMapLock) {
        AutoPtr<IDisplay> display;
        mDisplayManager->GetDisplay(displayId, (IDisplay**)&display);
        if (display != NULL) {
            CreateDisplayContentLocked(display);
            DisplayReady(displayId);
        }
        RequestTraversalLocked();
    }
}

ECode CWindowManagerService::OnDisplayRemoved(
    /* [in] */ Int32 displayId)
{
    AutoPtr<IMessage> msg;
    mH->ObtainMessage(H::DO_DISPLAY_REMOVED, displayId, 0, (IMessage**)&msg);
    Boolean result;
    return mH->SendMessage(msg, &result);
}

void CWindowManagerService::HandleDisplayRemovedLocked(
    /* [in] */ Int32 displayId)
{
    AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
    if (displayContent != NULL) {
        if (displayContent->IsAnimating()) {
            displayContent->mDeferredRemoval = TRUE;
            return;
        }
        if (DEBUG_DISPLAY) Slogger::V(TAG, "Removing display=%p", displayContent.Get());
        mDisplayContents->Delete(displayId);
        displayContent->Close();
        if (displayId == IDisplay::DEFAULT_DISPLAY) {
            UnregisterPointerEventListener(displayContent->mTapDetector);
        }
    }
    mAnimator->RemoveDisplayLocked(displayId);
    RequestTraversalLocked();
}

ECode CWindowManagerService::OnDisplayChanged(
    /* [in] */ Int32 displayId)
{
    AutoPtr<IMessage> msg;
    mH->ObtainMessage(H::DO_DISPLAY_CHANGED, displayId, 0, (IMessage**)&msg);
    Boolean result;
    return mH->SendMessage(msg, &result);
}

void CWindowManagerService::HandleDisplayChangedLocked(
    /* [in] */ Int32 displayId)
{
    AutoPtr<DisplayContent> displayContent = GetDisplayContentLocked(displayId);
    if (displayContent != NULL) {
        displayContent->UpdateDisplayInfo();
    }
    RequestTraversalLocked();
}

ECode CWindowManagerService::GetWindowManagerLock(
    /* [out] */ IInterface** lock)
{
    VALIDATE_NOT_NULL(lock)
    *lock = (IObject*)&mWindowMapLock;
    REFCOUNT_ADD(*lock)
    return NOERROR;
}

ECode CWindowManagerService::ToString(
    /* [ou] */ String* str)
{
    return Object::ToString(str);
}


} // Wm
} // Server
} // Droid
} // Elastos