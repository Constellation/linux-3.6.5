/*
  Copyright (C) 2014 Yusuke Suzuki <yusuke.suzuki@sslab.ics.keio.ac.jp>

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
#ifndef PVDRM_BACK_DRV_H_
#define PVDRM_BACK_DRV_H_
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/wait.h>

#include <asm/xen/hypervisor.h>

#include "../pvdrm/pvdrm_slot.h"
#include "../pvdrm/pvdrm_log.h"

struct pvdrm_back_device;
struct xenbus_device;

struct pvdrm_back_file {
	struct pvdrm_back_device* info;
	struct file* filp;
	int handle;
};

struct pvdrm_back_file* pvdrm_back_file_new(struct pvdrm_back_device* info);
void pvdrm_back_file_destroy(struct pvdrm_back_file* filp);

struct pvdrm_back_work {
	struct work_struct base;
	struct pvdrm_back_device* info;
	struct pvdrm_slot* slot;
};

struct pvdrm_back_device {
	struct xenbus_device* xbdev;
	struct task_struct* thread;
	struct pvdrm_back_file* file;
	grant_ref_t ref;
	struct pvdrm_mapped* mapped;
	atomic_t get;
	void* slot_addrs[PVDRM_SLOT_NR];
	struct pvdrm_back_work works[PVDRM_SLOT_NR];
	struct workqueue_struct* wq;
	bool sequential;
	struct list_head vmas;
	struct idr file_idr;
	spinlock_t file_lock;
};

struct pvdrm_back_vma {
	struct vm_area_struct base;
	struct list_head head;
	struct vm_struct* area;
	pte_t** pteps;
	int* refs;
	uint64_t map_handle;
	uint32_t handle;
	uint32_t pages;
	struct drm_gem_object* obj;
	struct pvdrm_back_device* info;
};

#endif  /* PVDRM_BACK_DRV_H_ */
/* vim: set sw=8 ts=8 et tw=80 : */