// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "orttraining/training_ops/cpu/nn/broadcast_grad_args.h"

namespace onnxruntime {
namespace contrib {
#define REGISTER_KERNEL_TYPED(T)                                  \
  ONNX_OPERATOR_TYPED_KERNEL_EX(                                  \
      BroadcastGradientArgs,                                      \
      kMSDomain,                                                  \
      1,                                                          \
      T,                                                          \
      kCpuExecutionProvider,                                      \
      KernelDefBuilder()                                          \
          .TypeConstraint("T", DataTypeImpl::GetTensorType<T>()), \
      BroadcastGradientArgs<T>);

REGISTER_KERNEL_TYPED(int64_t)

template <typename T>
Status BroadcastGradientArgs<T>::Compute(OpKernelContext* context) const {
  const Tensor* a_shape = context->Input<Tensor>(0);
  const Tensor* b_shape = context->Input<Tensor>(1);
  const T* A_dims = a_shape->template Data<T>();
  const T* B_dims = b_shape->template Data<T>();

  const int a_size = a_shape->Shape().Size();
  const int b_size = b_shape->Shape().Size();

  int ndim = std::max(a_size, b_size);
  std::vector<T> a_axes, b_axes;

  int i = int(a_size - 1);
  int j = int(b_size - 1);
  int k = ndim - 1;

  for (; i >= 0 && j >= 0; --k) {
    auto A_dim = A_dims[i],
         B_dim = B_dims[j];

    if (A_dim != B_dim) {
      if (A_dim == 1) {
        a_axes.push_back(gsl::narrow_cast<T>(k));
      }
      if (B_dim == 1) {
        b_axes.push_back(gsl::narrow_cast<T>(k));
      }
    }
    --i;
    --j;
  }

  if (i < 0) {
    for (; k >= 0; --k) {
      a_axes.push_back(gsl::narrow_cast<T>(k));
    }

  } else {
    for (; k >= 0; --k) {
      b_axes.push_back(gsl::narrow_cast<T>(k));
    }
  }

  Tensor* A_axes = context->Output(0, {static_cast<int64_t>(a_axes.size())});
  // if (A_axes) {
  T* A_axes_data = A_axes->template MutableData<T>();
  std::copy(a_axes.begin(), a_axes.end(), A_axes_data);
  // }

  Tensor* B_axes = context->Output(1, {static_cast<int64_t>(b_axes.size())});
  // if (B_axes) {
  T* B_axes_data = B_axes->template MutableData<T>();
  std::copy(b_axes.begin(), b_axes.end(), B_axes_data);
  // }
  if (!A_axes && !B_axes)
    ORT_THROW("No outputs available.");

  return Status::OK();
}

}  // namespace contrib
}  // namespace onnxruntime
