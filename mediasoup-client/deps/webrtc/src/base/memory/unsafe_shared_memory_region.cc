// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/unsafe_shared_memory_region.h"

#include <utility>

#include "base/memory/shared_memory.h"

namespace base {

// static
UnsafeSharedMemoryRegion UnsafeSharedMemoryRegion::Create(size_t size) {
  subtle::PlatformSharedMemoryRegion handle =
      subtle::PlatformSharedMemoryRegion::CreateUnsafe(size);

  return UnsafeSharedMemoryRegion(std::move(handle));
}

// static
UnsafeSharedMemoryRegion UnsafeSharedMemoryRegion::CreateFromHandle(
    const SharedMemoryHandle& handle) {
  if (!handle.IsValid())
    return UnsafeSharedMemoryRegion();
  auto platform_region =
      subtle::PlatformSharedMemoryRegion::TakeFromSharedMemoryHandle(
          handle, subtle::PlatformSharedMemoryRegion::Mode::kUnsafe);
  if (!platform_region.IsValid()) {
    return UnsafeSharedMemoryRegion();
  }
  return Deserialize(std::move(platform_region));
}

// static
UnsafeSharedMemoryRegion UnsafeSharedMemoryRegion::Deserialize(
    subtle::PlatformSharedMemoryRegion handle) {
  return UnsafeSharedMemoryRegion(std::move(handle));
}

// static
subtle::PlatformSharedMemoryRegion
UnsafeSharedMemoryRegion::TakeHandleForSerialization(
    UnsafeSharedMemoryRegion region) {
  return std::move(region.handle_);
}

UnsafeSharedMemoryRegion::UnsafeSharedMemoryRegion() = default;
UnsafeSharedMemoryRegion::UnsafeSharedMemoryRegion(
    UnsafeSharedMemoryRegion&& region) = default;
UnsafeSharedMemoryRegion& UnsafeSharedMemoryRegion::operator=(
    UnsafeSharedMemoryRegion&& region) = default;
UnsafeSharedMemoryRegion::~UnsafeSharedMemoryRegion() = default;

UnsafeSharedMemoryRegion UnsafeSharedMemoryRegion::Duplicate() const {
  return UnsafeSharedMemoryRegion(handle_.Duplicate());
}

WritableSharedMemoryMapping UnsafeSharedMemoryRegion::Map() const {
  return MapAt(0, handle_.GetSize());
}

WritableSharedMemoryMapping UnsafeSharedMemoryRegion::MapAt(off_t offset,
                                                            size_t size) const {
  if (!IsValid())
    return {};

  void* memory = nullptr;
  size_t mapped_size = 0;
  if (!handle_.MapAt(offset, size, &memory, &mapped_size))
    return {};

  return WritableSharedMemoryMapping(memory, size, mapped_size,
                                     handle_.GetGUID());
}

bool UnsafeSharedMemoryRegion::IsValid() const {
  return handle_.IsValid();
}

UnsafeSharedMemoryRegion::UnsafeSharedMemoryRegion(
    subtle::PlatformSharedMemoryRegion handle)
    : handle_(std::move(handle)) {
  if (handle_.IsValid()) {
    CHECK_EQ(handle_.GetMode(),
             subtle::PlatformSharedMemoryRegion::Mode::kUnsafe);
  }
}

}  // namespace base
