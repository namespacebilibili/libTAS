#include "opengl.h"

#ifdef LIBTAS_HUD

#include "../external/gl.h"

void (*glGenTextures_real)(int n, unsigned int* tex);
void (*glBindTexture_real)(int target, unsigned int tex);
void (*glTexImage2D_real)(int, int, int, int, int, int, int, int, const void*);
void (*glBegin_real)( int mode );
void (*glEnd_real)( void );
void (*glVertex2f_real)( float x, float y );
void (*glTexCoord2f_real)( float s, float t );
void (*glDeleteTextures_real)( int n, const unsigned int *textures);
void (*glEnable_real)( int cap );
void (*glDisable_real)( int cap );
void (*glVertexPointer_real)(int size, int type, int stride, const void* pointer);
void (*glDrawArrays_real)( int mode, int first, int count);

void (*glMatrixMode_real)(int mode);
void (*glPushMatrix_real)(void);
void (*glPopMatrix_real)(void);
void (*glLoadIdentity_real)(void);
void (*glOrtho_real)(double left, double right, double bottom, double top, double near, double far);
void (*glBlendFunc_real)(int sfactor, int dfactor);
void (*glTexParameteri_real)(int target, int pname, int param);
void (*glGetIntegerv_real)( int pname, GLint* data);
void (*glGetBooleanv_real)( int pname, GLboolean* data);

/* Link function pointers to real opengl functions */
void link_opengl(void);
{
    LINK_SUFFIX(glGenTextures, "libGL");
    LINK_SUFFIX(glBindTexture, "libGL");
    LINK_SUFFIX(glTexImage2D, "libGL");
    LINK_SUFFIX(glBegin, "libGL");
    LINK_SUFFIX(glEnd, "libGL");
    LINK_SUFFIX(glVertex2f, "libGL");
    LINK_SUFFIX(glTexCoord2f, "libGL");
    LINK_SUFFIX(glDeleteTextures, "libGL");
    LINK_SUFFIX(glEnable, "libGL");
    LINK_SUFFIX(glDisable, "libGL");
    LINK_SUFFIX(glVertexPointer, "libGL");
    LINK_SUFFIX(glDrawArrays, "libGL");
    LINK_SUFFIX(glMatrixMode, "libGL");
    LINK_SUFFIX(glPushMatrix, "libGL");
    LINK_SUFFIX(glPopMatrix, "libGL");
    LINK_SUFFIX(glLoadIdentity, "libGL");
    LINK_SUFFIX(glOrtho, "libGL");
    LINK_SUFFIX(glBlendFunc, "libGL");
    LINK_SUFFIX(glTexParameteri, "libGL");
    LINK_SUFFIX(glGetIntegerv, "libGL");
    LINK_SUFFIX(glGetBooleanv, "libGL");
}

/* Render a text on top of the game window 
 * Taken from http://stackoverflow.com/questions/5289447/using-sdl-ttf-with-opengl
 */
void RenderText(TTF_Font *font, const char* message, int sw, int sh, SDL_Color color, int x, int y) {
    glMatrixMode_real(GL_PROJECTION);
    glPushMatrix_real();
    glLoadIdentity_real();
    //glOrtho_real(0, sw, 0, sh, -1, 1); // m_Width and m_Height is the resolution of window
    glOrtho_real(0, sw, sh, 0, -1, 1); // m_Width and m_Height is the resolution of window

    glMatrixMode_real(GL_MODELVIEW);
    //glPushMatrix_real();
    glLoadIdentity_real();

    glDisable_real(GL_DEPTH_TEST);

    GLboolean tex2DEnabled;
    glGetBooleanv_real(GL_TEXTURE_2D, &tex2DEnabled);

    glEnable_real(GL_TEXTURE_2D);

    GLboolean blendEnabled;
    glGetBooleanv_real(GL_BLEND, &blendEnabled);

    GLint blendSrc;
    GLint blendDst;
    glGetIntegerv_real(GL_BLEND_SRC_ALPHA, &blendSrc);
    glGetIntegerv_real(GL_BLEND_DST_ALPHA, &blendDst);

    glEnable_real(GL_BLEND);
    glBlendFunc_real(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Get previous blind texture */
    GLint last_tex = 0;
    glGetIntegerv_real(GL_TEXTURE_BINDING_2D, &last_tex);

    /* Create text texture */
    //#if 0
    GLuint texture;
    glGenTextures_real(1, &texture);
    glBindTexture_real(GL_TEXTURE_2D, texture);

    SDL_Surface * sFont = TTF_RenderText_Blended(font, message, color);

    glTexParameteri_real(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri_real(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D_real(GL_TEXTURE_2D, 0, GL_RGBA, sFont->w, sFont->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, sFont->pixels);

    glBegin_real(GL_QUADS);
    {
        glTexCoord2f_real(0,0); glVertex2f_real(x, y);
        glTexCoord2f_real(1,0); glVertex2f_real(x + sFont->w, y);
        glTexCoord2f_real(1,1); glVertex2f_real(x + sFont->w, y + sFont->h);
        glTexCoord2f_real(0,1); glVertex2f_real(x, y + sFont->h);
    }
    glEnd_real();

    if (last_tex != 0) {
        glBindTexture_real(GL_TEXTURE_2D, last_tex);
    }

    //#endif

    if (! blendEnabled )
        glDisable_real(GL_BLEND);
    glBlendFunc_real(blendSrc, blendDst);

    if (! tex2DEnabled )
        glDisable_real(GL_TEXTURE_2D);

    //glEnable_real(GL_DEPTH_TEST);

    glMatrixMode_real(GL_PROJECTION);
    glPopMatrix_real();
    glMatrixMode_real(GL_MODELVIEW);
    //glPopMatrix_real();

    glDeleteTextures_real(1, &texture);
    SDL_FreeSurface_real(sFont);
}

#endif