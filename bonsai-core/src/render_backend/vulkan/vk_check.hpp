#pragma once
#ifndef BONSAI_RENDERER_VK_CHECK_HPP
#define BONSAI_RENDERER_VK_CHECK_HPP

#include <volk.h>

#define VK_SUCCEEDED(result)    ((result) == VK_SUCCESS)
#define VK_FAILED(result)       ((result) != VK_SUCCESS)

#endif //BONSAI_RENDERER_VK_CHECK_HPP