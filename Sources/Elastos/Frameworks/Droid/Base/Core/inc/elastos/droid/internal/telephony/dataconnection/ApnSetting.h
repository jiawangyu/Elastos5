
#ifndef __ELASTOS_DROID_INTERNAL_TELEPHONY_DATACONNECTION_APNSETTING_H__
#define __ELASTOS_DROID_INTERNAL_TELEPHONY_DATACONNECTION_APNSETTING_H__

#include "_Elastos.Droid.Internal.h"
#include <elastos/core/Object.h>
#include <elastos/droid/ext/frameworkext.h>

using Elastos::Utility::IList;

namespace Elastos {
namespace Droid {
namespace Internal {
namespace Telephony {
namespace DataConnection {

/**
 * This class represents a apn setting for create PDP link
 */
class ApnSetting
    : public Object
    , public IApnSetting
{
public:
    CAR_INTERFACE_DECL()

    ApnSetting();

    CARAPI constructor(
        /* [in] */ Int32 id,
        /* [in] */ const String& numeric,
        /* [in] */ const String& carrier,
        /* [in] */ const String& apn,
        /* [in] */ const String& proxy,
        /* [in] */ const String& port,
        /* [in] */ const String& mmsc,
        /* [in] */ const String& mmsProxy,
        /* [in] */ const String& mmsPort,
        /* [in] */ const String& user,
        /* [in] */ const String& password,
        /* [in] */ Int32 authType,
        /* [in] */ ArrayOf<String>* types,
        /* [in] */ const String& protocol,
        /* [in] */ const String& roamingProtocol,
        /* [in] */ Boolean carrierEnabled,
        /* [in] */ Int32 bearer,
        /* [in] */ Int32 profileId,
        /* [in] */ Boolean modemCognitive,
        /* [in] */ Int32 maxConns,
        /* [in] */ Int32 waitTime,
        /* [in] */ Int32 maxConnsTime,
        /* [in] */ Int32 mtu,
        /* [in] */ const String& mvnoType,
        /* [in] */ const String& mvnoMatchData);

    /**
     * Creates an ApnSetting object from a string.
     *
     * @param data the string to read.
     *
     * The string must be in one of two formats (newlines added for clarity,
     * spaces are optional):
     *
     * v1 format:
     *   <carrier>, <apn>, <proxy>, <port>, <user>, <password>, <server>,
     *   <mmsc>, <mmsproxy>, <mmsport>, <mcc>, <mnc>, <authtype>,
     *   <type>[| <type>...],
     *
     * v2 format:
     *   [ApnSettingV2] <carrier>, <apn>, <proxy>, <port>, <user>, <password>, <server>,
     *   <mmsc>, <mmsproxy>, <mmsport>, <mcc>, <mnc>, <authtype>,
     *   <type>[| <type>...], <protocol>, <roaming_protocol>, <carrierEnabled>, <bearer>,
     *
     * v3 format:
     *   [ApnSettingV3] <carrier>, <apn>, <proxy>, <port>, <user>, <password>, <server>,
     *   <mmsc>, <mmsproxy>, <mmsport>, <mcc>, <mnc>, <authtype>,
     *   <type>[| <type>...], <protocol>, <roaming_protocol>, <carrierEnabled>, <bearer>,
     *   <profileId>, <modemCognitive>, <maxConns>, <waitTime>, <maxConnsTime>, <mtu>,
     *   <mvnoType>, <mvnoMatchData>
     *
     * Note that the strings generated by toString() do not contain the username
     * and password and thus cannot be read by this method.
     */
    static CARAPI FromString(
        /* [in] */ const String& data,
        /* [out] */ IApnSetting** result);

    /**
     * Creates an array of ApnSetting objects from a string.
     *
     * @param data the string to read.
     *
     * Builds on top of the same format used by fromString, but allows for multiple entries
     * separated by "; ".
     */
    static CARAPI ArrayFromString(
        /* [in] */ const String& data,
        /* [out] */ IList** result);

    // @Override
    CARAPI ToString(
        /* [out] */ String* result);

    /**
     * Returns true if there are MVNO params specified.
     */
    CARAPI HasMvnoParams(
        /* [out] */ Boolean* result);

    CARAPI CanHandleType(
        /* [in] */ const String& type,
        /* [out] */ Boolean* result);

    // Also should handle changes in type order and perhaps case-insensitivity
    // @Override
    CARAPI Equals(
        /* [in] */ IInterface* o,
        /* [out] */ Boolean* result);

    CARAPI GetApnProfileType(
        /* [out] */ ApnProfileType* result);

    CARAPI GetProfileId(
        /* [out] */ Int32* result);

    CARAPI ToShortString(
        /* [out] */ String* result);

    CARAPI ToHash(
        /* [out] */ String* result);

    CARAPI GetCarrier(
        /* [out] */ String* result);

    CARAPI GetApn(
        /* [out] */ String* result);

    CARAPI GetProxy(
        /* [out] */ String* result);

    CARAPI GetPort(
        /* [out] */ String* result);

    CARAPI GetMmsc(
        /* [out] */ String* result);

    CARAPI GetMmsProxy(
        /* [out] */ String* result);

    CARAPI GetMmsPort(
        /* [out] */ String* result);

    CARAPI GetUser(
        /* [out] */ String* result);

    CARAPI GetPassword(
        /* [out] */ String* result);

    CARAPI GetAuthType(
        /* [out] */ Int32* result);

    CARAPI GetTypes(
        /* [out] */ ArrayOf<String>** result);

    CARAPI SetTypes(
        /* [in] */ ArrayOf<String>* types);

    CARAPI GetId(
        /* [out] */ Int32* result);

    CARAPI GetNumeric(
        /* [out] */ String* result);

    CARAPI GetProtocol(
        /* [out] */ String* result);

    CARAPI GetRoamingProtocol(
        /* [out] */ String* result);

    CARAPI GetMtu(
        /* [out] */ Int32* result);

    /**
      * Current status of APN
      * true : enabled APN, false : disabled APN.
      */
    CARAPI GetCarrierEnabled(
        /* [out] */ Boolean* result);

    /**
      * Radio Access Technology info
      * To check what values can hold, refer to ServiceState.java.
      * This should be spread to other technologies,
      * but currently only used for LTE(14) and EHRPD(13).
      */
    CARAPI GetBearer(
        /* [out] */ Int32* result);

    /* ID of the profile in the modem */
    CARAPI SetProfileId(
        /* [in] */ Int32 profileId);

    CARAPI GetModemCognitive(
        /* [out] */ Boolean* result);

    CARAPI GetMaxConns(
        /* [out] */ Int32* result);

    CARAPI GetWaitTime(
        /* [out] */ Int32* result);

    CARAPI GetMaxConnsTime(
        /* [out] */ Int32* result);

    /**
      * MVNO match type. Possible values:
      *   "spn": Service provider name.
      *   "imsi": IMSI.
      *   "gid": Group identifier level 1.
      */
    CARAPI GetMvnoType(
        /* [out] */ String* result);

    /**
      * MVNO data. Examples:
      *   "spn": A MOBILE, BEN NL
      *   "imsi": 302720x94, 2060188
      *   "gid": 4E, 33
      */
    CARAPI GetMvnoMatchData(
        /* [out] */ String* result);

public:
    static const String V2_FORMAT_REGEX;

    static const String V3_FORMAT_REGEX;

private:
    String mCarrier;

    String mApn;

    String mProxy;

    String mPort;

    String mMmsc;

    String mMmsProxy;

    String mMmsPort;

    String mUser;

    String mPassword;

    Int32 mAuthType;

    AutoPtr<ArrayOf<String> > mTypes;

    Int32 mId;

    String mNumeric;

    String mProtocol;

    String mRoamingProtocol;

    Int32 mMtu;

    /**
      * Current status of APN
      * true : enabled APN, false : disabled APN.
      */
    Boolean mCarrierEnabled;

    /**
      * Radio Access Technology info
      * To check what values can hold, refer to ServiceState.java.
      * This should be spread to other technologies,
      * but currently only used for LTE(14) and EHRPD(13).
      */
    Int32 mBearer;

    /* ID of the profile in the modem */
    Int32 mProfileId;

    Boolean mModemCognitive;

    Int32 mMaxConns;

    Int32 mWaitTime;

    Int32 mMaxConnsTime;

    /**
      * MVNO match type. Possible values:
      *   "spn": Service provider name.
      *   "imsi": IMSI.
      *   "gid": Group identifier level 1.
      */
    String mMvnoType;

    /**
      * MVNO data. Examples:
      *   "spn": A MOBILE, BEN NL
      *   "imsi": 302720x94, 2060188
      *   "gid": 4E, 33
      */
    String mMvnoMatchData;
};

} // namespace DataConnection
} // namespace Telephony
} // namespace Internal
} // namespace Droid
} // namespace Elastos

#endif // __ELASTOS_DROID_INTERNAL_TELEPHONY_DATACONNECTION_APNSETTING_H__