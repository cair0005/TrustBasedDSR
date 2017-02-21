/* -*- c++ -*-
   hdr_sr.cc
   source route header
*/

#include <stdio.h>
#include "hdr_sr.h"

int hdr_sr::offset_;

static class SRHeaderClass : public PacketHeaderClass {
public:
	SRHeaderClass() : PacketHeaderClass("PacketHeader/SR",
					     sizeof(hdr_sr)) {
		offset(&hdr_sr::offset_);

#ifdef DSR_CONST_HDR_SZ
		fprintf(stderr,"WARNING: DSR treating all source route headers\n"
			"as having length %d. this should be used only to estimate effect\n"
			"of no longer needing a src rt in each packet\n",SR_HDR_SZ);
#endif

	}
#if 0
	void export_offsets() {
		field_offset("valid_", OFFSET(hdr_sr, valid_));
		field_offset("num_addrs_", OFFSET(hdr_sr, num_addrs_));
		field_offset("cur_addr_", OFFSET(hdr_sr, cur_addr_));
	}
#endif
} class_SRhdr;

char *
hdr_sr::dump()
{
  static char buf[100];
  dump(buf);
  return (buf);
}

void
hdr_sr::dump(char *buf)
{
  char *ptr = buf;
  *ptr++ = '[';
  for (int i = 0; i < num_addrs_; i++)
    {
      ptr += sprintf(ptr, "%s%d ",
		     (i == cur_addr_) ? "|" : "",
		     addrs()[i].addr);
    }
  *ptr++ = ']';
  *ptr = '\0';
}
