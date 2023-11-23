/*
 *	The PCI Utilities -- Show Capabilities
 *
 *	Copyright (c) 1997--2018 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <string.h>

#include "adna.h"
#include "ls-caps.h"
#include "ls-ecaps.h"

static void cap_pm(struct device *d, int where, int cap)
{
  int t, b;
  (void)(b);
  (void)(cap);
  t = get_conf_word(d, where + PCI_PM_CTRL);
  printf("\tPower State: D%d\n", t & PCI_PM_CTRL_STATE_MASK); 
  // Ensure PM state is D0 (PM 04h[1:0]=0)
  if ((t & 0x3) != PCI_CAP_PM_STATE_D0)
    adna_set_d3_flag(d->NumDevice);
}

static inline char * ht_link_width(unsigned width)
{
  static char * const widths[8] = { "8bit", "16bit", "[2]", "32bit", "2bit", "4bit", "[6]", "N/C" };
  return widths[width];
}

static inline char * ht_link_freq(unsigned freq)
{
  static char * const freqs[16] = { "200MHz", "300MHz", "400MHz", "500MHz", "600MHz", "800MHz", "1.0GHz", "1.2GHz",
				    "1.4GHz", "1.6GHz", "[a]", "[b]", "[c]", "[d]", "[e]", "Vend" };
  return freqs[freq];
}

static int exp_downstream_port(int type)
{
  return type == PCI_EXP_TYPE_ROOT_PORT ||
	 type == PCI_EXP_TYPE_DOWNSTREAM ||
	 type == PCI_EXP_TYPE_PCIE_BRIDGE;	/* PCI/PCI-X to PCIe Bridge */
}

static char *link_speed(int speed)
{
  switch (speed)
    {
      case 1:
	return "2.5GT/s";
      case 2:
	return "5GT/s";
      case 3:
	return "8GT/s";
      case 4:
        return "16GT/s";
      case 5:
        return "32GT/s";
      case 6:
        return "64GT/s";
      default:
	return "unknown";
    }
}

static char *link_compare(int sta, int cap)
{
  if (sta < cap)
    return "downgraded";
  if (sta > cap)
    return "strange";
  return "ok";
}

static const char *aspm_enabled(int code)
{
  static const char *desc[] = { "Disabled", "L0s Enabled", "L1 Enabled", "L0s L1 Enabled" };
  return desc[code];
}

static void cap_express_link(struct device *d, int where, int type)
{
  (void)(type);
  u32 t, aspm, cap_speed, cap_width, sta_speed, sta_width;
  u16 w;

  t = get_conf_long(d, where + PCI_EXP_LNKCAP);
  aspm = (t & PCI_EXP_LNKCAP_ASPM) >> 10;
  cap_speed = t & PCI_EXP_LNKCAP_SPEED;
  cap_width = (t & PCI_EXP_LNKCAP_WIDTH) >> 4;
  (void)(aspm);

  w = get_conf_word(d, where + PCI_EXP_LNKCTL);
  printf("\tASPM: ASPM %s\n", aspm_enabled(w & PCI_EXP_LNKCTL_ASPM));  // Custom

  w = get_conf_word(d, where + PCI_EXP_LNKSTA);
  sta_speed = w & PCI_EXP_LNKSTA_SPEED;
  sta_width = (w & PCI_EXP_LNKSTA_WIDTH) >> 4;
  printf("\tLink Status: Speed %s (%s), Width x%d (%s)\n",
	link_speed(sta_speed),
	link_compare(sta_speed, cap_speed),
	sta_width,
	link_compare(sta_width, cap_width));

  printf("\tLinkActive: %s\n", FLAG(w, PCI_EXP_LNKSTA_DL_ACT) == '+' ? "Yes" : "No");
}

static void cap_express_slot(struct device *d, int where)
{
  u32 t;
  u16 w;

  t = get_conf_long(d, where + PCI_EXP_SLTCAP);

printf("\tHotplug: Hotplug %s\n", FLAG(t, PCI_EXP_SLTCAP_HPC) == '+' ? "Supported" : "Not Supported"); // Custom
  w = get_conf_word(d, where + PCI_EXP_SLTCTL);
  w = get_conf_word(d, where + PCI_EXP_SLTSTA);
  (void)(w);
}

static void cap_express_root(struct device *d, int where)
{
  u32 w;

  w = get_conf_word(d, where + PCI_EXP_RTCAP);
  printf("\t\tRootCap: CRSVisible%c\n",
	FLAG(w, PCI_EXP_RTCAP_CRSVIS));

  w = get_conf_word(d, where + PCI_EXP_RTCTL);
  printf("\t\tRootCtl: ErrCorrectable%c ErrNon-Fatal%c ErrFatal%c PMEIntEna%c CRSVisible%c\n",
	FLAG(w, PCI_EXP_RTCTL_SECEE),
	FLAG(w, PCI_EXP_RTCTL_SENFEE),
	FLAG(w, PCI_EXP_RTCTL_SEFEE),
	FLAG(w, PCI_EXP_RTCTL_PMEIE),
	FLAG(w, PCI_EXP_RTCTL_CRSVIS));

  w = get_conf_long(d, where + PCI_EXP_RTSTA);
  printf("\t\tRootSta: PME ReqID %04x, PMEStatus%c PMEPending%c\n",
	w & PCI_EXP_RTSTA_PME_REQID,
	FLAG(w, PCI_EXP_RTSTA_PME_STATUS),
	FLAG(w, PCI_EXP_RTSTA_PME_PENDING));
}

static const char *cap_express_dev2_timeout_range(int type)
{
  /* Decode Completion Timeout Ranges. */
  switch (type)
    {
      case 0:
	return "Not Supported";
      case 1:
	return "Range A";
      case 2:
	return "Range B";
      case 3:
	return "Range AB";
      case 6:
	return "Range BC";
      case 7:
	return "Range ABC";
      case 14:
	return "Range BCD";
      case 15:
	return "Range ABCD";
      default:
	return "Unknown";
    }
}

static const char *cap_express_dev2_timeout_value(int type)
{
  /* Decode Completion Timeout Value. */
  switch (type)
    {
      case 0:
	return "50us to 50ms";
      case 1:
	return "50us to 100us";
      case 2:
	return "1ms to 10ms";
      case 5:
	return "16ms to 55ms";
      case 6:
	return "65ms to 210ms";
      case 9:
	return "260ms to 900ms";
      case 10:
	return "1s to 3.5s";
      case 13:
	return "4s to 13s";
      case 14:
	return "17s to 64s";
      default:
	return "Unknown";
    }
}

static const char *cap_express_devcap2_obff(int obff)
{
  switch (obff)
    {
      case 1:
        return "Via message";
      case 2:
        return "Via WAKE#";
      case 3:
        return "Via message/WAKE#";
      default:
        return "Not Supported";
    }
}

static const char *cap_express_devcap2_epr(int epr)
{
  switch (epr)
    {
      case 1:
        return "Dev Specific";
      case 2:
        return "Form Factor Dev Specific";
      case 3:
        return "Reserved";
      default:
        return "Not Supported";
    }
}

static const char *cap_express_devcap2_lncls(int lncls)
{
  switch (lncls)
    {
      case 1:
        return "64byte cachelines";
      case 2:
        return "128byte cachelines";
      case 3:
        return "Reserved";
      default:
        return "Not Supported";
    }
}

static const char *cap_express_devcap2_tphcomp(int tph)
{
  switch (tph)
    {
      case 1:
        return "TPHComp+ ExtTPHComp-";
      case 2:
        /* Reserved; intentionally left blank */
        return "";
      case 3:
        return "TPHComp+ ExtTPHComp+";
      default:
        return "TPHComp- ExtTPHComp-";
    }
}

static const char *cap_express_devctl2_obff(int obff)
{
  switch (obff)
    {
      case 0:
        return "Disabled";
      case 1:
        return "Via message A";
      case 2:
        return "Via message B";
      case 3:
        return "Via WAKE#";
      default:
        return "Unknown";
    }
}

static int device_has_memory_space_bar(struct device *d)
{
  struct pci_dev *p = d->dev;
  int i, found = 0;

  for (i=0; i<6; i++)
    if (p->base_addr[i] && p->size[i])
      {
        if (!(p->base_addr[i] & PCI_BASE_ADDRESS_SPACE_IO))
          {
            found = 1;
            break;
          }
      }
  return found;
}

static void cap_express_dev2(struct device *d, int where, int type)
{
  u32 l;
  u16 w;
  int has_mem_bar = device_has_memory_space_bar(d);

  l = get_conf_long(d, where + PCI_EXP_DEVCAP2);
  printf("\t\tDevCap2: Completion Timeout: %s, TimeoutDis%c NROPrPrP%c LTR%c",
        cap_express_dev2_timeout_range(PCI_EXP_DEVCAP2_TIMEOUT_RANGE(l)),
        FLAG(l, PCI_EXP_DEVCAP2_TIMEOUT_DIS),
	FLAG(l, PCI_EXP_DEVCAP2_NROPRPRP),
        FLAG(l, PCI_EXP_DEVCAP2_LTR));
  printf("\n\t\t\t 10BitTagComp%c 10BitTagReq%c OBFF %s, ExtFmt%c EETLPPrefix%c",
        FLAG(l, PCI_EXP_DEVCAP2_10BIT_TAG_COMP),
        FLAG(l, PCI_EXP_DEVCAP2_10BIT_TAG_REQ),
        cap_express_devcap2_obff(PCI_EXP_DEVCAP2_OBFF(l)),
        FLAG(l, PCI_EXP_DEVCAP2_EXTFMT),
        FLAG(l, PCI_EXP_DEVCAP2_EE_TLP));

  if (PCI_EXP_DEVCAP2_EE_TLP == (l & PCI_EXP_DEVCAP2_EE_TLP))
    {
      printf(", MaxEETLPPrefixes %d",
             PCI_EXP_DEVCAP2_MEE_TLP(l) ? PCI_EXP_DEVCAP2_MEE_TLP(l) : 4);
    }

  printf("\n\t\t\t EmergencyPowerReduction %s, EmergencyPowerReductionInit%c",
        cap_express_devcap2_epr(PCI_EXP_DEVCAP2_EPR(l)),
        FLAG(l, PCI_EXP_DEVCAP2_EPR_INIT));
  printf("\n\t\t\t FRS%c", FLAG(l, PCI_EXP_DEVCAP2_FRS));

  if (type == PCI_EXP_TYPE_ROOT_PORT)
    printf(" LN System CLS %s,",
          cap_express_devcap2_lncls(PCI_EXP_DEVCAP2_LN_CLS(l)));

  if (type == PCI_EXP_TYPE_ROOT_PORT || type == PCI_EXP_TYPE_ENDPOINT)
    printf(" %s", cap_express_devcap2_tphcomp(PCI_EXP_DEVCAP2_TPH_COMP(l)));

  if (type == PCI_EXP_TYPE_ROOT_PORT || type == PCI_EXP_TYPE_DOWNSTREAM)
    printf(" ARIFwd%c\n", FLAG(l, PCI_EXP_DEVCAP2_ARI));
  else
    printf("\n");
  if (type == PCI_EXP_TYPE_ROOT_PORT || type == PCI_EXP_TYPE_UPSTREAM ||
      type == PCI_EXP_TYPE_DOWNSTREAM || has_mem_bar)
    {
       printf("\t\t\t AtomicOpsCap:");
       if (type == PCI_EXP_TYPE_ROOT_PORT || type == PCI_EXP_TYPE_UPSTREAM ||
           type == PCI_EXP_TYPE_DOWNSTREAM)
         printf(" Routing%c", FLAG(l, PCI_EXP_DEVCAP2_ATOMICOP_ROUTING));
       if (type == PCI_EXP_TYPE_ROOT_PORT || has_mem_bar)
         printf(" 32bit%c 64bit%c 128bitCAS%c",
		FLAG(l, PCI_EXP_DEVCAP2_32BIT_ATOMICOP_COMP),
		FLAG(l, PCI_EXP_DEVCAP2_64BIT_ATOMICOP_COMP),
		FLAG(l, PCI_EXP_DEVCAP2_128BIT_CAS_COMP));
       printf("\n");
    }

  w = get_conf_word(d, where + PCI_EXP_DEVCTL2);
  printf("\t\tDevCtl2: Completion Timeout: %s, TimeoutDis%c LTR%c 10BitTagReq%c OBFF %s,",
	cap_express_dev2_timeout_value(PCI_EXP_DEVCTL2_TIMEOUT_VALUE(w)),
	FLAG(w, PCI_EXP_DEVCTL2_TIMEOUT_DIS),
	FLAG(w, PCI_EXP_DEVCTL2_LTR),
	FLAG(w, PCI_EXP_DEVCTL2_10BIT_TAG_REQ),
	cap_express_devctl2_obff(PCI_EXP_DEVCTL2_OBFF(w)));
  if (type == PCI_EXP_TYPE_ROOT_PORT || type == PCI_EXP_TYPE_DOWNSTREAM)
    printf(" ARIFwd%c\n", FLAG(w, PCI_EXP_DEVCTL2_ARI));
  else
    printf("\n");
  if (type == PCI_EXP_TYPE_ROOT_PORT || type == PCI_EXP_TYPE_UPSTREAM ||
      type == PCI_EXP_TYPE_DOWNSTREAM || type == PCI_EXP_TYPE_ENDPOINT ||
      type == PCI_EXP_TYPE_ROOT_INT_EP || type == PCI_EXP_TYPE_LEG_END)
    {
      printf("\t\t\t AtomicOpsCtl:");
      if (type == PCI_EXP_TYPE_ROOT_PORT || type == PCI_EXP_TYPE_ENDPOINT ||
          type == PCI_EXP_TYPE_ROOT_INT_EP || type == PCI_EXP_TYPE_LEG_END)
        printf(" ReqEn%c", FLAG(w, PCI_EXP_DEVCTL2_ATOMICOP_REQUESTER_EN));
      if (type == PCI_EXP_TYPE_ROOT_PORT || type == PCI_EXP_TYPE_UPSTREAM ||
          type == PCI_EXP_TYPE_DOWNSTREAM)
        printf(" EgressBlck%c", FLAG(w, PCI_EXP_DEVCTL2_ATOMICOP_EGRESS_BLOCK));
      printf("\n");
    }
}

static const char *cap_express_link2_speed_cap(int vector)
{
  /*
   * Per PCIe r5.0, sec 8.2.1, a device must support 2.5GT/s and is not
   * permitted to skip support for any data rates between 2.5GT/s and the
   * highest supported rate.
   */
  if (vector & 0x60)
    return "RsvdP";
  if (vector & 0x10)
    return "2.5-32GT/s";
  if (vector & 0x08)
    return "2.5-16GT/s";
  if (vector & 0x04)
    return "2.5-8GT/s";
  if (vector & 0x02)
    return "2.5-5GT/s";
  if (vector & 0x01)
    return "2.5GT/s";

  return "Unknown";
}

static const char *cap_express_link2_speed(int type)
{
  switch (type)
    {
      case 0: /* hardwire to 0 means only the 2.5GT/s is supported */
      case 1:
	return "2.5GT/s";
      case 2:
	return "5GT/s";
      case 3:
	return "8GT/s";
      case 4:
        return "16GT/s";
      case 5:
        return "32GT/s";
      case 6:
        return "64GT/s";
      default:
	return "Unknown";
    }
}

static const char *cap_express_link2_deemphasis(int type)
{
  switch (type)
    {
      case 0:
	return "-6dB";
      case 1:
	return "-3.5dB";
      default:
	return "Unknown";
    }
}

static const char *cap_express_link2_transmargin(int type)
{
  switch (type)
    {
      case 0:
	return "Normal Operating Range";
      case 1:
	return "800-1200mV(full-swing)/400-700mV(half-swing)";
      case 2:
      case 3:
      case 4:
      case 5:
	return "200-400mV(full-swing)/100-200mV(half-swing)";
      default:
	return "Unknown";
    }
}

static const char *cap_express_link2_crosslink_res(int crosslink)
{
  switch (crosslink)
    {
      case 0:
        return "unsupported";
      case 1:
        return "Upstream Port";
      case 2:
        return "Downstream Port";
      default:
        return "incomplete";
    }
}

static const char *cap_express_link2_component(int presence)
{
  switch (presence)
    {
      case 0:
        return "Link Down - Not Determined";
      case 1:
        return "Link Down - Not Present";
      case 2:
        return "Link Down - Present";
      case 4:
        return "Link Up - Present";
      case 5:
        return "Link Up - Present and DRS Received";
      default:
        return "Reserved";
    }
}

static void cap_express_link2(struct device *d, int where, int type)
{
  u32 l = 0;
  u16 w;

  if (!((type == PCI_EXP_TYPE_ENDPOINT || type == PCI_EXP_TYPE_LEG_END) &&
	(d->dev->dev != 0 || d->dev->func != 0))) {
    /* Link Capabilities 2 was reserved before PCIe r3.0 */
    l = get_conf_long(d, where + PCI_EXP_LNKCAP2);
    if (l) {
      printf("\t\tLnkCap2: Supported Link Speeds: %s, Crosslink%c "
	"Retimer%c 2Retimers%c DRS%c\n",
	  cap_express_link2_speed_cap(PCI_EXP_LNKCAP2_SPEED(l)),
	  FLAG(l, PCI_EXP_LNKCAP2_CROSSLINK),
	  FLAG(l, PCI_EXP_LNKCAP2_RETIMER),
	  FLAG(l, PCI_EXP_LNKCAP2_2RETIMERS),
	  FLAG(l, PCI_EXP_LNKCAP2_DRS));
    }

    w = get_conf_word(d, where + PCI_EXP_LNKCTL2);
    printf("\t\tLnkCtl2: Target Link Speed: %s, EnterCompliance%c SpeedDis%c",
	cap_express_link2_speed(PCI_EXP_LNKCTL2_SPEED(w)),
	FLAG(w, PCI_EXP_LNKCTL2_CMPLNC),
	FLAG(w, PCI_EXP_LNKCTL2_SPEED_DIS));
    if (type == PCI_EXP_TYPE_DOWNSTREAM)
      printf(", Selectable De-emphasis: %s",
	cap_express_link2_deemphasis(PCI_EXP_LNKCTL2_DEEMPHASIS(w)));
    printf("\n"
	"\t\t\t Transmit Margin: %s, EnterModifiedCompliance%c ComplianceSOS%c\n"
	"\t\t\t Compliance De-emphasis: %s\n",
	cap_express_link2_transmargin(PCI_EXP_LNKCTL2_MARGIN(w)),
	FLAG(w, PCI_EXP_LNKCTL2_MOD_CMPLNC),
	FLAG(w, PCI_EXP_LNKCTL2_CMPLNC_SOS),
	cap_express_link2_deemphasis(PCI_EXP_LNKCTL2_COM_DEEMPHASIS(w)));
  }

  w = get_conf_word(d, where + PCI_EXP_LNKSTA2);
  printf("\t\tLnkSta2: Current De-emphasis Level: %s, EqualizationComplete%c EqualizationPhase1%c\n"
	"\t\t\t EqualizationPhase2%c EqualizationPhase3%c LinkEqualizationRequest%c\n"
	"\t\t\t Retimer%c 2Retimers%c CrosslinkRes: %s",
	cap_express_link2_deemphasis(PCI_EXP_LINKSTA2_DEEMPHASIS(w)),
	FLAG(w, PCI_EXP_LINKSTA2_EQU_COMP),
	FLAG(w, PCI_EXP_LINKSTA2_EQU_PHASE1),
	FLAG(w, PCI_EXP_LINKSTA2_EQU_PHASE2),
	FLAG(w, PCI_EXP_LINKSTA2_EQU_PHASE3),
	FLAG(w, PCI_EXP_LINKSTA2_EQU_REQ),
	FLAG(w, PCI_EXP_LINKSTA2_RETIMER),
	FLAG(w, PCI_EXP_LINKSTA2_2RETIMERS),
	cap_express_link2_crosslink_res(PCI_EXP_LINKSTA2_CROSSLINK(w)));

  if (exp_downstream_port(type) && (l & PCI_EXP_LNKCAP2_DRS)) {
    printf(", DRS%c\n"
	"\t\t\t DownstreamComp: %s\n",
	FLAG(w, PCI_EXP_LINKSTA2_DRS_RCVD),
	cap_express_link2_component(PCI_EXP_LINKSTA2_COMPONENT(w)));
  } else
    printf("\n");
}

static void cap_express_slot2(struct device *d UNUSED, int where UNUSED)
{
  /* No capabilities that require this field in PCIe rev2.0 spec. */
}

static int cap_express(struct device *d, int where, int cap)
{
  int type = (cap & PCI_EXP_FLAGS_TYPE) >> 4;
  int size;
  int slot = 0;
  int link = 1;

  printf("\tPort Type: ");
  // printf("Express ");
  // if (verbose >= 2)
  //   printf("(v%d) ", cap & PCI_EXP_FLAGS_VERS);
  switch (type)
    {
    case PCI_EXP_TYPE_ENDPOINT:
      printf("Endpoint");
      break;
    case PCI_EXP_TYPE_LEG_END:
      printf("Legacy Endpoint");
      break;
    case PCI_EXP_TYPE_ROOT_PORT:
      slot = cap & PCI_EXP_FLAGS_SLOT;
      printf("Root Port (Slot%c)", FLAG(cap, PCI_EXP_FLAGS_SLOT));
      break;
    case PCI_EXP_TYPE_UPSTREAM:
      printf("Upstream Port");
      break;
    case PCI_EXP_TYPE_DOWNSTREAM:
      slot = cap & PCI_EXP_FLAGS_SLOT;
      // printf("Downstream Port (Slot%c)", FLAG(cap, PCI_EXP_FLAGS_SLOT));
      printf("Downstream Port (No EEPROM access)");
      break;
    case PCI_EXP_TYPE_PCI_BRIDGE:
      printf("PCI-Express to PCI/PCI-X Bridge");
      break;
    case PCI_EXP_TYPE_PCIE_BRIDGE:
      slot = cap & PCI_EXP_FLAGS_SLOT;
      printf("PCI/PCI-X to PCI-Express Bridge (Slot%c)",
	     FLAG(cap, PCI_EXP_FLAGS_SLOT));
      break;
    case PCI_EXP_TYPE_ROOT_INT_EP:
      link = 0;
      printf("Root Complex Integrated Endpoint");
      break;
    case PCI_EXP_TYPE_ROOT_EC:
      link = 0;
      printf("Root Complex Event Collector");
      break;
    default:
      printf("Unknown type %d", type);
  }
  // printf(", MSI %02x\n", (cap & PCI_EXP_FLAGS_IRQ) >> 9);
  printf("\n");
  if (verbose < 2)
    return type;

  size = 16;
  if (slot)
    size = 24;
  if (type == PCI_EXP_TYPE_ROOT_PORT || type == PCI_EXP_TYPE_ROOT_EC)
    size = 32;
  if (!config_fetch(d, where + PCI_EXP_DEVCAP, size))
    return type;
  if (link)
    cap_express_link(d, where, type);
  if (slot)
    cap_express_slot(d, where);
  return type; // Custom, Early out
  if (type == PCI_EXP_TYPE_ROOT_PORT || type == PCI_EXP_TYPE_ROOT_EC)
    cap_express_root(d, where);

  if ((cap & PCI_EXP_FLAGS_VERS) < 2)
    return type;

  size = 16;
  if (slot)
    size = 24;
  if (!config_fetch(d, where + PCI_EXP_DEVCAP2, size))
    return type;

  cap_express_dev2(d, where, type);
  if (link)
    cap_express_link2(d, where, type);
  if (slot)
    cap_express_slot2(d, where);
  return type;
}

void show_caps(struct device *d, int where)
{
  int can_have_ext_caps = 0;
  int type = -1;

  if (get_conf_word(d, PCI_STATUS) & PCI_STATUS_CAP_LIST)
  {
    byte been_there[256];
    where = get_conf_byte(d, where) & ~3;
    memset(been_there, 0, 256);
    while (where)
    {
      int id, next, cap;

      if (!config_fetch(d, where, 4))
      {
        puts("<access denied>");
        break;
      }
      id = get_conf_byte(d, where + PCI_CAP_LIST_ID);
      // if (PCI_CAP_ID_EXP == id) {
      //   printf("\tCapabilities: ");
      // }
      next = get_conf_byte(d, where + PCI_CAP_LIST_NEXT) & ~3;
      cap = get_conf_word(d, where + PCI_CAP_FLAGS);
      // if (PCI_CAP_ID_EXP == id)
      //   printf("[%02x] ", where);

      if (been_there[where]++)
      {
        printf("<chain looped>\n");
        break;
      }
      if (id == 0xff)
      {
        printf("<chain broken>\n");
        break;
      }

      switch (id)
      {

      case PCI_CAP_ID_NULL:
        printf("Null\n");
        break;
      case PCI_CAP_ID_PM:
        cap_pm(d, where, cap);
        break;
      case PCI_CAP_ID_EXP:
        type = cap_express(d, where, cap);
        can_have_ext_caps = 1;
        break;
      default:
        break;
      }
      where = next;
    }
  }
  if (can_have_ext_caps)
    show_ext_caps(d, type);
}
