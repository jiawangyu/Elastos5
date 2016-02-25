
#include "elastos/droid/wifi/CBatchedScanSettings.h"
#include <elastos/core/StringBuffer.h>
#include <elastos/core/StringUtils.h>

using Elastos::Core::ICharSequence;
using Elastos::Core::StringBuffer;
using Elastos::Core::StringUtils;
using Elastos::Utility::CArrayList;
using Elastos::Utility::IArrayList;
using Elastos::Utility::IIterator;

namespace Elastos {
namespace Droid {
namespace Wifi {

CAR_INTERFACE_IMPL_2(CBatchedScanSettings, Object, IBatchedScanSettings, IParcelable)

CAR_OBJECT_IMPL(CBatchedScanSettings)

CBatchedScanSettings::CBatchedScanSettings()
    : mMaxScansPerBatch(0)
    , mMaxApPerScan(0)
    , mScanIntervalSec(0)
    , mMaxApForDistance(0)
{
}

ECode CBatchedScanSettings::constructor()
{
    return Clear();
}

ECode CBatchedScanSettings::constructor(
    /* [in] */ IBatchedScanSettings* source)
{
    source->GetMaxScansPerBatch(&mMaxScansPerBatch);
    source->GetMaxApPerScan(&mMaxApPerScan);
    AutoPtr<ICollection> channelSet;
    source->GetChannelSet((ICollection**)&mChannelSet);
    if (channelSet != NULL) {
        AutoPtr<ICollection> cs;
        source->GetChannelSet((ICollection**)&cs);
        CArrayList::New(cs, (IArrayList**)&mChannelSet);
    }
    source->GetScanIntervalSec(&mScanIntervalSec);
    return source->GetMaxApForDistance(&mMaxApForDistance);
}

ECode CBatchedScanSettings::GetMaxScansPerBatch(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mMaxScansPerBatch;
    return NOERROR;
}

ECode CBatchedScanSettings::SetMaxScansPerBatch(
    /* [in] */ Int32 maxScansPerBatch)
{
    mMaxScansPerBatch = maxScansPerBatch;
    return NOERROR;
}

ECode CBatchedScanSettings::GetMaxApPerScan(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mMaxApPerScan;
    return NOERROR;
}

ECode CBatchedScanSettings::SetMaxApPerScan(
    /* [in] */ Int32 maxApPerScan)
{
    mMaxApPerScan = maxApPerScan;
    return NOERROR;
}

ECode CBatchedScanSettings::GetChannelSet(
    /* [out] */ ICollection** result)
{
    VALIDATE_NOT_NULL(result);
    *result = mChannelSet;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode CBatchedScanSettings::SetChannelSet(
    /* [in] */ ICollection* channelSet)
{
    mChannelSet = channelSet;
    return NOERROR;
}

ECode CBatchedScanSettings::GetScanIntervalSec(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mScanIntervalSec;
    return NOERROR;
}

ECode CBatchedScanSettings::SetScanIntervalSec(
    /* [in] */ Int32 scanIntervalSec)
{
    mScanIntervalSec = scanIntervalSec;
    return NOERROR;
}

ECode CBatchedScanSettings::GetMaxApForDistance(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mMaxApForDistance;
    return NOERROR;
}

ECode CBatchedScanSettings::SetMaxApForDistance(
    /* [in] */ Int32 maxApForDistance)
{
    mMaxApForDistance = maxApForDistance;
    return NOERROR;
}

ECode CBatchedScanSettings::Clear()
{
    mMaxScansPerBatch = UNSPECIFIED;
    mMaxApPerScan = UNSPECIFIED;
    mChannelSet = NULL;
    mScanIntervalSec = UNSPECIFIED;
    mMaxApForDistance = UNSPECIFIED;
    return NOERROR;
}

/** @hide */
ECode CBatchedScanSettings::IsInvalid(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    if (mMaxScansPerBatch != UNSPECIFIED && (mMaxScansPerBatch < MIN_SCANS_PER_BATCH ||
            mMaxScansPerBatch > MAX_SCANS_PER_BATCH)) {
        *result = TRUE;
        return NOERROR;
    }

    if (mMaxApPerScan != UNSPECIFIED && (mMaxApPerScan < MIN_AP_PER_SCAN ||
            mMaxApPerScan > MAX_AP_PER_SCAN)) {
        *result = TRUE;
        return NOERROR;
    }

    if (ChannelSetIsValid() == FALSE) {
        *result = TRUE;
        return NOERROR;
    }

    if (mScanIntervalSec != UNSPECIFIED && (mScanIntervalSec < MIN_INTERVAL_SEC ||
            mScanIntervalSec > MAX_INTERVAL_SEC)) {
        *result = TRUE;
        return NOERROR;
    }

    if (mMaxApForDistance != UNSPECIFIED && (mMaxApForDistance < MIN_AP_FOR_DISTANCE ||
            mMaxApForDistance > MAX_AP_FOR_DISTANCE)) {
        *result = TRUE;
        return NOERROR;
    }

    *result = FALSE;

    return NOERROR;
}

/** @hide */
ECode CBatchedScanSettings::Constrain()
{
    if (mScanIntervalSec == UNSPECIFIED) {
        mScanIntervalSec = DEFAULT_INTERVAL_SEC;
    }
    else if (mScanIntervalSec < MIN_INTERVAL_SEC) {
        mScanIntervalSec = MIN_INTERVAL_SEC;
    }
    else if (mScanIntervalSec > MAX_INTERVAL_SEC) {
        mScanIntervalSec = MAX_INTERVAL_SEC;
    }

    if (mMaxScansPerBatch == UNSPECIFIED) {
        mMaxScansPerBatch = DEFAULT_SCANS_PER_BATCH;
    }
    else if (mMaxScansPerBatch < MIN_SCANS_PER_BATCH) {
        mMaxScansPerBatch = MIN_SCANS_PER_BATCH;
    }
    else if (mMaxScansPerBatch > MAX_SCANS_PER_BATCH) {
        mMaxScansPerBatch = MAX_SCANS_PER_BATCH;
    }

    if (mMaxApPerScan == UNSPECIFIED) {
        mMaxApPerScan = DEFAULT_AP_PER_SCAN;
    }
    else if (mMaxApPerScan < MIN_AP_PER_SCAN) {
        mMaxApPerScan = MIN_AP_PER_SCAN;
    }
    else if (mMaxApPerScan > MAX_AP_PER_SCAN) {
        mMaxApPerScan = MAX_AP_PER_SCAN;
    }

    if (mMaxApForDistance == UNSPECIFIED) {
        mMaxApForDistance = DEFAULT_AP_FOR_DISTANCE;
    }
    else if (mMaxApForDistance < MIN_AP_FOR_DISTANCE) {
        mMaxApForDistance = MIN_AP_FOR_DISTANCE;
    }
    else if (mMaxApForDistance > MAX_AP_FOR_DISTANCE) {
        mMaxApForDistance = MAX_AP_FOR_DISTANCE;
    }

    return NOERROR;
}

//@Override
ECode CBatchedScanSettings::Equals(
    /* [in] */ IObject* obj,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    if (IBatchedScanSettings::Probe(obj) == NULL) {
        *result = FALSE;
        return NOERROR;
    }

    AutoPtr<IBatchedScanSettings> o = IBatchedScanSettings::Probe(obj);
    Int32 _maxScansPerBatch, _maxApPerScan, _scanIntervalSec, _maxApForDistance;
    o->GetMaxScansPerBatch(&_maxScansPerBatch);
    if (mMaxScansPerBatch != _maxScansPerBatch ||
          (o->GetMaxApPerScan(&_maxApPerScan), mMaxApPerScan != _maxApPerScan) ||
          (o->GetScanIntervalSec(&_scanIntervalSec), mScanIntervalSec != _scanIntervalSec) ||
          (o->GetMaxApForDistance(&_maxApForDistance), mMaxApForDistance != _maxApForDistance)) {
        *result = FALSE;
        return NOERROR;
    }

    AutoPtr<ICollection> _channelSet;
    o->GetChannelSet((ICollection**)&_channelSet);
    if (mChannelSet == NULL) {
        *result = (_channelSet == NULL);
        return NOERROR;
    }

    return mChannelSet->Equals(_channelSet, result);
}

//@Override
ECode CBatchedScanSettings::HashCode(
    /* [out] */ Int32* code)
{
    VALIDATE_NOT_NULL(code);

    Int32 _code;
    assert(0);
    // TODO
    // mChannelSet->HashCode(&_code);
    // *code = mMaxScansPerBatch +
    //             (mMaxApPerScan * 3) +
    //             (mScanIntervalSec * 5) +
    //             (mMaxApForDistance * 7) +
    //             (_code * 11);

    return NOERROR;
}

//@Override
ECode CBatchedScanSettings::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str);

    StringBuffer sb;
    String none("<none>");

    sb.Append("BatchScanSettings [maxScansPerBatch: ");
    if (mMaxScansPerBatch == UNSPECIFIED) {
        sb.Append(none);
    }
    else {
        sb.Append(mMaxScansPerBatch);
    }
    sb.Append(", maxApPerScan: ");
    if (mMaxScansPerBatch == UNSPECIFIED) {
        sb.Append(none);
    }
    else {
        sb.Append(mMaxApPerScan);
    }
    sb.Append(", scanIntervalSec: ");
    if (mMaxScansPerBatch == UNSPECIFIED) {
        sb.Append(none);
    }
    else {
        sb.Append(mScanIntervalSec);
    }
    sb.Append(", maxApForDistance: ");
    if (mMaxScansPerBatch == UNSPECIFIED) {
        sb.Append(none);
    }
    else {
        sb.Append(mMaxApForDistance);
    }
    sb.Append(", channelSet: ");
    if (mChannelSet == NULL) {
        sb.Append("ALL");
    }
    else {
        sb.Append("<");
        AutoPtr<IIterator> iter;
        mChannelSet->GetIterator((IIterator**)&iter);
        Boolean bNext;
        iter->HasNext(&bNext);
        for (; bNext; iter->HasNext(&bNext)) {
            AutoPtr<ICharSequence> channel;
            iter->GetNext((IInterface**)&channel);
            String str;
            channel->ToString(&str);
            sb.Append(" ");
            sb.Append(str);
        }
        sb.Append(">");
    }
    sb.Append("]");

    return sb.ToString(str);
}

/** Implement the Parcelable interface {@hide} */
ECode CBatchedScanSettings::DescribeContents(
    /* [out] */ Int32* contents)
{
    VALIDATE_NOT_NULL(contents);
    *contents = 0;
    return NOERROR;
}

/** Implement the Parcelable interface {@hide} */
ECode CBatchedScanSettings::WriteToParcel(
    /* [in] */ IParcel* dest,
    /* [in] */ Int32 flags)
{
    dest->WriteInt32(mMaxScansPerBatch);
    dest->WriteInt32(mMaxApPerScan);
    dest->WriteInt32(mScanIntervalSec);
    dest->WriteInt32(mMaxApForDistance);
    Int32 size;
    mChannelSet->GetSize(&size);
    dest->WriteInt32(mChannelSet == NULL ? 0 : size);
    if (mChannelSet != NULL) {
        AutoPtr<IIterator> iter;
        mChannelSet->GetIterator((IIterator**)&iter);
        Boolean bNext;
        iter->HasNext(&bNext);
        for (; bNext; iter->HasNext(&bNext)) {
            AutoPtr<ICharSequence> channel;
            iter->GetNext((IInterface**)&channel);
            String str;
            channel->ToString(&str);
            dest->WriteString(str);
        }
    }

    return NOERROR;
}

ECode CBatchedScanSettings::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    assert(0);
    // TODO
    return E_NOT_IMPLEMENTED;
}

ECode CBatchedScanSettings::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    assert(0);
    // TODO
    return E_NOT_IMPLEMENTED;
}

Boolean CBatchedScanSettings::ChannelSetIsValid()
{
    Boolean bIsEmpty;
    if (mChannelSet == NULL || (mChannelSet->IsEmpty(&bIsEmpty), bIsEmpty)) {
        return TRUE;
    }

    AutoPtr<IIterator> iter;
    mChannelSet->GetIterator((IIterator**)&iter);
    Boolean bNext;
    iter->HasNext(&bNext);
    for (; bNext; iter->HasNext(&bNext)) {
        AutoPtr<ICharSequence> iChannel;
        iter->GetNext((IInterface**)&iChannel);
        String channel;
        iChannel->ToString(&channel);
        // try {
            Int32 i = StringUtils::ParseInt32(channel);
            if (i > 0 && i <= MAX_WIFI_CHANNEL) continue;
        // } catch (NumberFormatException e) {}
        if (channel.Equals("A") || channel.Equals("B")) continue;
        return FALSE;
    }

    return TRUE;
}

} // namespace Wifi
} // namespace Droid
} // namespace Elastos