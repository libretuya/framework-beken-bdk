/*
 * hostapd / Station table
 * Copyright (c) 2002-2011, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */
#ifndef STA_INFO_H
#define STA_INFO_H

#ifdef CONFIG_MESH
/* needed for mesh_plink_state enum */
#include "common/defs.h"
#include "common/wpa_common.h"
#endif /* CONFIG_MESH */

#include "list.h"
#include "vlan.h"

/* STA flags */
#define WLAN_STA_AUTH BIT(0)
#define WLAN_STA_ASSOC BIT(1)
#define WLAN_STA_AUTHORIZED BIT(5)
#define WLAN_STA_PENDING_POLL BIT(6) /* pending activity poll not ACKed */
#define WLAN_STA_SHORT_PREAMBLE BIT(7)
#define WLAN_STA_PREAUTH BIT(8)
#define WLAN_STA_WMM BIT(9)
#define WLAN_STA_MFP BIT(10)
#define WLAN_STA_HT BIT(11)
#define WLAN_STA_WPS BIT(12)
#define WLAN_STA_MAYBE_WPS BIT(13)
#define WLAN_STA_WDS BIT(14)
#define WLAN_STA_ASSOC_REQ_OK BIT(15)
#define WLAN_STA_WPS2 BIT(16)
#define WLAN_STA_GAS BIT(17)
#define WLAN_STA_VHT BIT(18)
#define WLAN_STA_WNM_SLEEP_MODE BIT(19)
#define WLAN_STA_VHT_OPMODE_ENABLED BIT(20)
#define WLAN_STA_VENDOR_VHT BIT(21)
#define WLAN_STA_PENDING_DISASSOC_CB BIT(29)
#define WLAN_STA_PENDING_DEAUTH_CB BIT(30)
#define WLAN_STA_NONERP BIT(31)

/* Maximum number of supported rates (from both Supported Rates and Extended
 * Supported Rates IEs). */
#define WLAN_SUPP_RATES_MAX 32


struct mbo_non_pref_chan_info {
	struct mbo_non_pref_chan_info *next;
	u8 op_class;
	u8 pref;
	u8 reason_code;
	u8 num_channels;
	u8 channels[];
};

struct pending_eapol_rx {
	struct wpabuf *buf;
	struct os_reltime rx_time;
};

struct sta_info {
	struct sta_info *next; /* next entry in sta list */
	struct sta_info *hnext; /* next entry in hash table list */
	u8 addr[6];
	be32 ipaddr;
	struct dl_list ip6addr; /* list head for struct ip6addr */
	u16 aid; /* STA's unique AID (1 .. 2007) or 0 if not yet assigned */
	u32 flags; /* Bitfield of WLAN_STA_* */
	u16 capability;
	u16 listen_interval; /* or beacon_int for APs */
	u8 supported_rates[WLAN_SUPP_RATES_MAX];
	int supported_rates_len;
	u8 qosinfo; /* Valid when WLAN_STA_WMM is set */

#ifdef CONFIG_MESH
	enum mesh_plink_state plink_state;
	u16 peer_lid;
	u16 my_lid;
	u16 peer_aid;
	u16 mpm_close_reason;
	int mpm_retries;
	u8 my_nonce[WPA_NONCE_LEN];
	u8 peer_nonce[WPA_NONCE_LEN];
	u8 aek[32];	/* SHA256 digest length */
	u8 mtk[WPA_TK_MAX_LEN];
	size_t mtk_len;
	u8 mgtk_rsc[6];
	u8 mgtk_key_id;		/* FIXME: Where to get Key ID? */
	u8 mgtk[WPA_TK_MAX_LEN];
	size_t mgtk_len;
	u8 igtk_rsc[6];
	u8 igtk[WPA_TK_MAX_LEN];
	size_t igtk_len;
	u16 igtk_key_id;
	u8 sae_auth_retry;

	u8 sta_idx;	/* FIXME: no here */
#endif /* CONFIG_MESH */

	unsigned int nonerp_set:1;
	unsigned int no_short_slot_time_set:1;
	unsigned int no_short_preamble_set:1;
	unsigned int no_ht_gf_set:1;
	unsigned int no_ht_set:1;
	unsigned int ht40_intolerant_set:1;
	unsigned int ht_20mhz_set:1;
	unsigned int no_p2p_set:1;
	unsigned int qos_map_enabled:1;
	unsigned int remediation:1;
	unsigned int hs20_deauth_requested:1;
	unsigned int session_timeout_set:1;
	unsigned int radius_das_match:1;
	unsigned int ecsa_supported:1;
	unsigned int added_unassoc:1;

	u16 auth_alg;

	enum {
		STA_NULLFUNC = 0, STA_DISASSOC, STA_DEAUTH, STA_REMOVE,
		STA_DISASSOC_FROM_CLI
	} timeout_next;

	u16 deauth_reason;
	u16 disassoc_reason;

	/* IEEE 802.1X related data */
	struct eapol_state_machine *eapol_sm;
	u8 *challenge; /* IEEE 802.11 Shared Key Authentication Challenge */

	struct wpa_state_machine *wpa_sm;

	int vlan_id; /* 0: none, >0: VID */
	struct vlan_description *vlan_desc;
	int vlan_id_bound; /* updated by ap_sta_bind_vlan() */
	 /* PSKs from RADIUS authentication server */
	struct hostapd_sta_wpa_psk_short *psk;

	struct ieee80211_ht_capabilities *ht_capabilities;
	struct ieee80211_vht_capabilities *vht_capabilities;
	u8 vht_opmode;

#ifdef CONFIG_IEEE80211W
	int sa_query_count; /* number of pending SA Query requests;
			     * 0 = no SA Query in progress */
	int sa_query_timed_out;
	u8 *sa_query_trans_id; /* buffer of WLAN_SA_QUERY_TR_ID_LEN *
				* sa_query_count octets of pending SA Query
				* transaction identifiers */
	struct os_reltime sa_query_start;
#endif /* CONFIG_IEEE80211W */

	struct wpabuf *wps_ie; /* WPS IE from (Re)Association Request */
	struct wpabuf *p2p_ie; /* P2P IE from (Re)Association Request */

	struct os_reltime connected_time;	/* accouting */

#ifdef CONFIG_SAE
	struct sae_data *sae;
	unsigned int mesh_sae_pmksa_caching:1;
#endif /* CONFIG_SAE */

	u32 session_timeout; /* valid only if session_timeout_set == 1 */

	/* Last Authentication/(Re)Association Request/Action frame sequence
	 * control */
	u16 last_seq_ctrl;
	/* Last Authentication/(Re)Association Request/Action frame subtype */
	u8 last_subtype;

	u8 *supp_op_classes; /* Supported Operating Classes element, if
			      * received, starting from the Length field */

	u8 rrm_enabled_capa[5];
};


/* Default value for maximum station inactivity. After AP_MAX_INACTIVITY has
 * passed since last received frame from the station, a nullfunc data frame is
 * sent to the station. If this frame is not acknowledged and no other frames
 * have been received, the station will be disassociated after
 * AP_DISASSOC_DELAY seconds. Similarly, the station will be deauthenticated
 * after AP_DEAUTH_DELAY seconds has passed after disassociation. */
#define AP_MAX_INACTIVITY        (20)
#define AP_DISASSOC_DELAY        (1)
#define AP_DEAUTH_DELAY          (1)
/* Number of seconds to keep STA entry with Authenticated flag after it has
 * been disassociated. */
#define AP_MAX_INACTIVITY_AFTER_DISASSOC (1 * 30)
/* Number of seconds to keep STA entry after it has been deauthenticated. */
#define AP_MAX_INACTIVITY_AFTER_DEAUTH (1 * 5)


struct hostapd_data;

int ap_for_each_sta(struct hostapd_data *hapd,
		    int (*cb)(struct hostapd_data *hapd, struct sta_info *sta,
			      void *ctx),
		    void *ctx);
struct sta_info * ap_get_sta(struct hostapd_data *hapd, const u8 *sta);
struct sta_info * ap_get_sta_p2p(struct hostapd_data *hapd, const u8 *addr);
void ap_sta_hash_add(struct hostapd_data *hapd, struct sta_info *sta);
void ap_free_sta(struct hostapd_data *hapd, struct sta_info *sta);
void ap_sta_ip6addr_del(struct hostapd_data *hapd, struct sta_info *sta);
void hostapd_free_stas(struct hostapd_data *hapd);
void ap_handle_timer(void *eloop_ctx, void *timeout_ctx);
void ap_sta_replenish_timeout(struct hostapd_data *hapd, struct sta_info *sta,
			      u32 session_timeout);
void ap_sta_session_timeout(struct hostapd_data *hapd, struct sta_info *sta,
			    u32 session_timeout);
void ap_sta_no_session_timeout(struct hostapd_data *hapd,
			       struct sta_info *sta);
void ap_sta_session_warning_timeout(struct hostapd_data *hapd,
				    struct sta_info *sta, int warning_time);
struct sta_info * ap_sta_add(struct hostapd_data *hapd, const u8 *addr);
void ap_sta_disassociate(struct hostapd_data *hapd, struct sta_info *sta,
			 u16 reason);
void ap_sta_deauthenticate(struct hostapd_data *hapd, struct sta_info *sta,
			   u16 reason);
#ifdef CONFIG_WPS
int ap_sta_wps_cancel(struct hostapd_data *hapd,
		      struct sta_info *sta, void *ctx);
#endif /* CONFIG_WPS */
int ap_sta_bind_vlan(struct hostapd_data *hapd, struct sta_info *sta);
int ap_sta_set_vlan(struct hostapd_data *hapd, struct sta_info *sta,
		    struct vlan_description *vlan_desc);
void ap_sta_start_sa_query(struct hostapd_data *hapd, struct sta_info *sta);
void ap_sta_stop_sa_query(struct hostapd_data *hapd, struct sta_info *sta);
int ap_check_sa_query_timeout(struct hostapd_data *hapd, struct sta_info *sta);
void ap_sta_disconnect(struct hostapd_data *hapd, struct sta_info *sta,
		       const u8 *addr, u16 reason);

void ap_sta_set_authorized(struct hostapd_data *hapd,
			   struct sta_info *sta, int authorized);
static inline int ap_sta_is_authorized(struct sta_info *sta)
{
	return sta->flags & WLAN_STA_AUTHORIZED;
}

void ap_sta_deauth_cb(struct hostapd_data *hapd, struct sta_info *sta);
void ap_sta_disassoc_cb(struct hostapd_data *hapd, struct sta_info *sta);
void ap_sta_clear_disconnect_timeouts(struct hostapd_data *hapd,
				      struct sta_info *sta);

int ap_sta_flags_txt(u32 flags, char *buf, size_t buflen);

#endif /* STA_INFO_H */