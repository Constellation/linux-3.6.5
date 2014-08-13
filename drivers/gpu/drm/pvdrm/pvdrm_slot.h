/*
  Copyright (C) 2014 Yusuke Suzuki <utatane.tea@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef PVDRM_SLOT_H_
#define PVDRM_SLOT_H_

#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <drm/nouveau_drm.h>

#include <xen/grant_table.h>
#define PVDRM_SLOT_NR 64

#include "../nouveau/nouveau_abi16.h"

#include "pvdrm_fence.h"

enum {
	PVDRM_IOCTL_NOUVEAU_GETPARAM,
	/* PVDRM_IOCTL_NOUVEAU_SETPARAM, */
	PVDRM_IOCTL_NOUVEAU_CHANNEL_ALLOC,
	PVDRM_IOCTL_NOUVEAU_CHANNEL_FREE,
	PVDRM_IOCTL_NOUVEAU_GROBJ_ALLOC,
	PVDRM_IOCTL_NOUVEAU_NOTIFIEROBJ_ALLOC,
	PVDRM_IOCTL_NOUVEAU_GPUOBJ_FREE,
	PVDRM_IOCTL_NOUVEAU_GEM_NEW,
	PVDRM_IOCTL_NOUVEAU_GEM_PUSHBUF,
	PVDRM_IOCTL_NOUVEAU_GEM_CPU_PREP,
	PVDRM_IOCTL_NOUVEAU_GEM_CPU_FINI,
	PVDRM_IOCTL_NOUVEAU_GEM_INFO,
};

struct pvdrm_slot {
	int __id;
	struct pvdrm_fence __fence;
	uint32_t code;
	union {
		uint64_t __payload;  /* To calculate palyload address */

		/* ioctl */
		struct drm_nouveau_getparam getparam;
		/* struct drm_nouveau_setparam setparam; */
		struct drm_nouveau_channel_alloc channel_alloc;
		struct drm_nouveau_channel_free channel_free;
		struct drm_nouveau_grobj_alloc grobj_alloc;
		struct drm_nouveau_notifierobj_alloc notifierobj_alloc;
		struct drm_nouveau_gpuobj_free gpuobj_free;
		struct drm_nouveau_gem_new gem_new;
		struct drm_nouveau_gem_pushbuf gem_pushbuf;
		struct drm_nouveau_gem_cpu_prep gem_cpu_prep;
		struct drm_nouveau_gem_cpu_fini gem_cpu_fini;
		struct drm_nouveau_gem_info gem_info;
	};
};

static inline void* pvdrm_slot_payload(struct pvdrm_slot* slot) {
	return ((uint8_t*)slot) + offsetof(struct pvdrm_slot, __payload);
}

struct pvdrm_slots {
	struct semaphore sema;
	spinlock_t lock;
	struct pvdrm_slot* entries[PVDRM_SLOT_NR];
	struct page* pages[PVDRM_SLOT_NR];
	grant_ref_t handles[PVDRM_SLOT_NR];
	uint8_t used[PVDRM_SLOT_NR];
};

struct pvdrm_device;

int pvdrm_slot_init(struct pvdrm_device* pvdrm);
struct pvdrm_slot* pvdrm_slot_alloc(struct pvdrm_device* pvdrm);
void pvdrm_slot_free(struct pvdrm_device* pvdrm, struct pvdrm_slot* slot);
int pvdrm_slot_request(struct pvdrm_device* pvdrm, struct pvdrm_slot* slot);

#endif  /* PVDRM_SLOT_H_ */
/* vim: set sw=8 ts=8 et tw=80 : */