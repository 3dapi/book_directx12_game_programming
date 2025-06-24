#pragma once
#ifndef _G2Constants_H_
#define _G2Constants_H_

#include <any>
#include <string>

enum
{
	FRAME_BUFFER_COUNT = 2,
};

struct IG2AppFrame
{
	virtual ~IG2AppFrame() = default;

	static IG2AppFrame* instance();
	virtual int init(const std::any& initialValue = {}) = 0;
};

enum EG2GRAPHICS
{
	EG2_GRAPHICS_NONE = 0,
	EG2_D3D9 = 9,		EG2_D3D10 = 11,		EG2_D3D11 = 11,		EG2_D3D12 = 12,
	EG2_OGL_2 = 20,		EG2_OGL_30 = 30,	EG2_OGL_31 = 31,	EG2_OGL_32 = 32,
	EG2_VULKAN1 = 41,	EG2_VULKAN1_2 = 42,	EG2_VULKAN1_3 = 43,
	EG2_METAL = 81,
};

struct IG2Graphics
{
	virtual ~IG2Graphics() = default;

	static IG2Graphics* instance();
	virtual EG2GRAPHICS type() const = 0;
	virtual int init(const std::any& initialValue = {}) = 0;

	virtual std::any getAttrib(int nAttrib) = 0;
	virtual int      setAttrib(int nAttrib, const std::any& v = {}) = 0;
	virtual int      command(int nCmd, const std::any& v = {}) = 0;
};

#endif // _G2Constants_H_


