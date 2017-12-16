#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OPENGL_

#include "irrTypes.h"
#include "COpenGLCubemapTexture.h"
#include "COpenGLDriver.h"


#include "irrString.h"

namespace irr
{
namespace video
{


COpenGLCubemapTexture::COpenGLCubemapTexture(GLenum internalFormat, const uint32_t* size, uint32_t mipmapLevels, const io::path& name) : COpenGLFilterableTexture(name,getOpenGLTextureType())
{
#ifdef _DEBUG
	setDebugName("COpenGLCubemapTexture");
#endif
    TextureSize[0] = size[0];
    TextureSize[1] = size[0];
    TextureSize[2] = 6;
    MipLevelsStored = mipmapLevels;

    InternalFormat = internalFormat;
    COpenGLExtensionHandler::extGlTextureStorage2D(TextureName,GL_TEXTURE_CUBE_MAP,MipLevelsStored,InternalFormat,TextureSize[0],TextureSize[1]);

    ColorFormat = getColorFormatFromSizedOpenGLFormat(InternalFormat);
}

bool COpenGLCubemapTexture::updateSubRegion(const ECOLOR_FORMAT &inDataColorFormat, const void* data, const uint32_t* minimum, const uint32_t* maximum, int32_t mipmap, const uint32_t& unpackRowByteAlignment)
{
    /*
    if (minimum[3]!=maximum[3])
    {
#ifdef _DEBUG
        os::Printer::log(ELL_ERROR,"Can only update one face of the cubemap at a time!");
#endif // _DEBUG
        return false;
    }
    */
    GLenum pixFmt,pixType;
	GLenum pixFmtSized = getOpenGLFormatAndParametersFromColorFormat(inDataColorFormat, pixFmt, pixType);
    bool sourceCompressed = COpenGLTexture::isInternalFormatCompressed(pixFmtSized);

    bool destinationCompressed = COpenGLTexture::isInternalFormatCompressed(InternalFormat);
    if ((!destinationCompressed)&&sourceCompressed)
        return false;

    if (destinationCompressed&&(minimum[0]||minimum[1]||maximum[0]!=TextureSize[0]||maximum[1]!=TextureSize[1]))
        return false;

    if (sourceCompressed)
    {
        size_t levelByteSize = (maximum[2]-minimum[2]);
        levelByteSize *= ((((maximum[0]-minimum[0])/(0x1u<<mipmap)+3)&0xfffffc)*(((maximum[1]-minimum[1])/(0x1u<<mipmap)+3)&0xfffffc)*COpenGLTexture::getOpenGLFormatBpp(InternalFormat))/8;

        //! FIX THIS
        COpenGLExtensionHandler::extGlCompressedTextureSubImage3D(TextureName,GL_TEXTURE_CUBE_MAP, mipmap, minimum[0],minimum[1],minimum[2], maximum[0]-minimum[0],maximum[1]-minimum[1],maximum[2]-minimum[2], InternalFormat, levelByteSize, data);
    }
    else
    {
        //! we're going to have problems with uploading lower mip levels
        uint32_t bpp = video::getBitsPerPixelFromFormat(inDataColorFormat);
        uint32_t pitchInBits = ((maximum[0]-minimum[0])>>mipmap)*bpp/8;

        COpenGLExtensionHandler::setPixelUnpackAlignment(pitchInBits,const_cast<void*>(data),unpackRowByteAlignment);
        //! FIX THIS
        COpenGLExtensionHandler::extGlTextureSubImage3D(TextureName,GL_TEXTURE_CUBE_MAP, mipmap, minimum[0],minimum[1],minimum[2], maximum[0]-minimum[0],maximum[1]-minimum[1],maximum[2]-minimum[2], pixFmt, pixType, data);
    }
    return true;
}

bool COpenGLCubemapTexture::resize(const uint32_t* size, const uint32_t& mipLevels)
{
    if (TextureSize[0]==size[0])
        return true;

    recreateName(getOpenGLTextureType());

    uint32_t defaultMipMapCount = 1u+uint32_t(floorf(log2(float(size[0]))));
    if (MipLevelsStored>1)
    {
        if (mipLevels==0)
            MipLevelsStored = defaultMipMapCount;
        else
            MipLevelsStored = core::min_(mipLevels,defaultMipMapCount);
    }

    TextureSize[0] = size[0];
    TextureSize[1] = size[0];
    COpenGLExtensionHandler::extGlTextureStorage2D(TextureName,GL_TEXTURE_CUBE_MAP,MipLevelsStored,InternalFormat,TextureSize[0],TextureSize[1]);

    return true;
}

}
}
#endif

