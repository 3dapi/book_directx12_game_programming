#include "G2.ConstantsWin.h"

namespace G2 {

IG2AppFrameWin* IG2AppFrameWin::instance()
{
	return dynamic_cast<IG2AppFrameWin*>(IG2AppFrame::instance());
}

PENGINE_D3D IG2GraphicsD3D::instance()
{
	return dynamic_cast<PENGINE_D3D>(IG2Graphics::instance());
}

} // namespace G2
