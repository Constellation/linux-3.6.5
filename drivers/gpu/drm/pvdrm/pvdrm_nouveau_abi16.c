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

#include <linux/console.h>
#include <linux/device.h>
#include <linux/module.h>
#include <drm/nouveau_drm.h>

#include "drmP.h"
#include "drm.h"
#include "drm_crtc_helper.h"

/* Include nouveau's abi16 header directly. */
#include "../nouveau/nouveau_abi16.h"

#include "pvdrm_cast.h"
#include "pvdrm_channel.h"
#include "pvdrm_gem.h"
#include "pvdrm_slot.h"
#include "pvdrm_nouveau_abi16.h"

int pvdrm_nouveau_abi16_ioctl(struct drm_device *dev, int code, void *data, size_t size)
{
	struct pvdrm_device* pvdrm;
	int ret;
	pvdrm = drm_device_to_pvdrm(dev);
	{
		struct pvdrm_slot* slot = pvdrm_slot_alloc(pvdrm);
		slot->code = (code);
		memcpy(pvdrm_slot_payload(slot), data, size);
		ret = pvdrm_slot_request(pvdrm, slot);
		memcpy(data, pvdrm_slot_payload(slot), size);
		pvdrm_slot_free(pvdrm, slot);
	}
	return ret;
}

int pvdrm_nouveau_abi16_ioctl_getparam(struct drm_device *dev, void *data, struct drm_file *file)
{
	return pvdrm_nouveau_abi16_ioctl(dev, PVDRM_IOCTL_NOUVEAU_GETPARAM, data, sizeof(struct drm_nouveau_getparam));
}

int pvdrm_nouveau_abi16_ioctl_setparam(struct drm_device *dev, void *data, struct drm_file *file)
{
	return -EINVAL;
}

int pvdrm_nouveau_abi16_ioctl_channel_alloc(struct drm_device *dev, void *data, struct drm_file *file)
{
	struct drm_pvdrm_gem_object* result = NULL;
	return pvdrm_channel_alloc(dev, file, data, &result);
}

int pvdrm_nouveau_abi16_ioctl_channel_free(struct drm_device *dev, void *data, struct drm_file *file)
{
	/* FIXME: Not implemented yet. */
	return pvdrm_nouveau_abi16_ioctl(dev, PVDRM_IOCTL_NOUVEAU_CHANNEL_FREE, data, sizeof(struct drm_nouveau_channel_free));
}

int pvdrm_nouveau_abi16_ioctl_grobj_alloc(struct drm_device *dev, void *data, struct drm_file *file)
{
	return pvdrm_nouveau_abi16_ioctl(dev, PVDRM_IOCTL_NOUVEAU_GROBJ_ALLOC, data, sizeof(struct drm_nouveau_grobj_alloc));
}

int pvdrm_nouveau_abi16_ioctl_notifierobj_alloc(struct drm_device *dev, void *data, struct drm_file *file)
{
	return pvdrm_nouveau_abi16_ioctl(dev, PVDRM_IOCTL_NOUVEAU_NOTIFIEROBJ_ALLOC, data, sizeof(struct drm_nouveau_notifierobj_alloc));
}

int pvdrm_nouveau_abi16_ioctl_gpuobj_free(struct drm_device *dev, void *data, struct drm_file *file)
{
	return pvdrm_nouveau_abi16_ioctl(dev, PVDRM_IOCTL_NOUVEAU_GPUOBJ_FREE, data, sizeof(struct drm_nouveau_gpuobj_free));
}

int pvdrm_nouveau_gem_ioctl_new(struct drm_device *dev, void *data, struct drm_file *file)
{
	struct drm_pvdrm_gem_object* result = NULL;
	return pvdrm_gem_object_new(dev, file, data, &result);
}

struct pushbuf_copier {
	struct drm_nouveau_gem_pushbuf_bo* buffers;
	uint32_t nr_buffers;
	struct drm_nouveau_gem_pushbuf_reloc* relocs;
	uint32_t nr_relocs;
	struct drm_nouveau_gem_pushbuf_push* push;
	uint32_t nr_push;
};

int pvdrm_nouveau_gem_ioctl_pushbuf(struct drm_device *dev, void *data, struct drm_file *file)
{
	struct drm_nouveau_gem_pushbuf* req = data;
	struct drm_pvdrm_gem_object* chan;

	struct pushbuf_copier copier = {
		.buffers    = (void*)req->buffers,
		.nr_buffers = req->nr_buffers,
		.relocs     = (void*)req->relocs,
		.nr_relocs  = req->nr_relocs,
		.push       = (void*)req->push,
		.nr_push    = req->nr_push
	};

	chan = pvdrm_gem_object_lookup(dev, file, req->channel);
	if (!chan) {
		return -EINVAL;
	}
	req->channel = chan->host;

	if (req->nr_buffers && req->buffers) {
		if (req->nr_buffers > NOUVEAU_GEM_MAX_BUFFERS) {
			return -EINVAL;
		}
	}

	if (req->nr_relocs && req->relocs) {
		if (req->nr_relocs > NOUVEAU_GEM_MAX_RELOCS) {
			return -EINVAL;
		}
	}

	if (req->nr_push && req->push) {
		if (req->nr_push > NOUVEAU_GEM_MAX_PUSH) {
			return -EINVAL;
		}
	}

	return pvdrm_nouveau_abi16_ioctl(dev, PVDRM_IOCTL_NOUVEAU_GEM_PUSHBUF, data, sizeof(struct drm_nouveau_gem_pushbuf));
}

int pvdrm_nouveau_gem_ioctl_cpu_prep(struct drm_device *dev, void *data, struct drm_file *file)
{
	struct drm_nouveau_gem_cpu_prep* req = data;
	uint32_t handle = req->handle;
	struct drm_pvdrm_gem_object* obj;
	int ret;

	obj = pvdrm_gem_object_lookup(dev, file, handle);
	if (!obj) {
		return -EINVAL;
	}
	req->handle = obj->host;

	ret = pvdrm_nouveau_abi16_ioctl(dev, PVDRM_IOCTL_NOUVEAU_GEM_CPU_PREP, data, sizeof(struct drm_nouveau_gem_cpu_prep));

	req->handle = obj->handle;
	return ret;
}

int pvdrm_nouveau_gem_ioctl_cpu_fini(struct drm_device *dev, void *data, struct drm_file *file)
{
	struct drm_nouveau_gem_cpu_fini* req = data;
	uint32_t handle = req->handle;
	struct drm_pvdrm_gem_object* obj;
	int ret;

	obj = pvdrm_gem_object_lookup(dev, file, handle);
	if (!obj) {
		return -EINVAL;
	}
	req->handle = obj->host;

	ret = pvdrm_nouveau_abi16_ioctl(dev, PVDRM_IOCTL_NOUVEAU_GEM_CPU_FINI, data, sizeof(struct drm_nouveau_gem_cpu_fini));

	req->handle = obj->handle;
	return ret;
}

int pvdrm_nouveau_gem_ioctl_info(struct drm_device *dev, void *data, struct drm_file *file)
{
	struct drm_nouveau_gem_info* req = data;
	uint32_t handle = req->handle;
	struct drm_pvdrm_gem_object* obj;
	int ret;

	obj = pvdrm_gem_object_lookup(dev, file, handle);
	if (!obj) {
		return -EINVAL;
	}
	req->handle = obj->host;

	ret = pvdrm_nouveau_abi16_ioctl(dev, PVDRM_IOCTL_NOUVEAU_GEM_INFO, data, sizeof(struct drm_nouveau_gem_info));

	req->handle = obj->handle;
	return ret;
}

/* vim: set sw=8 ts=8 et tw=80 : */
