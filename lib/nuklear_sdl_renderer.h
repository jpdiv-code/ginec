/*
 * Nuklear - 1.32.0 - public domain
 * no warranty implied; use at your own risk.
 * authored from 2015-2016 by Micha Mettke
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_SDL_RENDERER_H_
#define NK_SDL_RENDERER_H_

#include <SDL2/SDL.h>

NK_API struct nk_context* nk_sdl_init(SDL_Window* win, SDL_Renderer* renderer);
NK_API void nk_sdl_font_stash_begin(struct nk_font_atlas** atlas);
NK_API void nk_sdl_font_stash_end(void);

NK_API int nk_sdl_handle_event(SDL_Event* evt);
NK_API void nk_sdl_render(enum nk_anti_aliasing);
NK_API void nk_sdl_shutdown(void);

#endif /* NK_SDL_RENDERER_H_ */

/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_SDL_RENDERER_IMPLEMENTATION

#include <string.h>

struct nk_sdl_device
{
    struct nk_buffer cmds;
    struct nk_draw_null_texture tex_null;
    SDL_Texture* font_tex;
};

struct nk_sdl_vertex
{
    float position[2];
    float uv[2];
    nk_byte col[4];
};

static struct nk_sdl
{
    SDL_Window* win;
    SDL_Renderer* renderer;
    struct nk_sdl_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
} sdl;

NK_INTERN void nk_sdl_device_upload_atlas(const void* image, int width, int height)
{
    struct nk_sdl_device* dev = &sdl.ogl;

    SDL_Texture* tex = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STATIC, width, height);
    if (!tex)
        return;

    SDL_UpdateTexture(tex, NULL, image, width * 4);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    dev->font_tex = tex;
}

NK_API void nk_sdl_render(enum nk_anti_aliasing AA)
{
    (void)AA; // Unused parameter
    struct nk_sdl_device* dev = &sdl.ogl;
    struct nk_buffer vbuf, ebuf;
    SDL_Rect viewport;
    const struct nk_draw_command* cmd;
    const nk_draw_index* offset = NULL;

    SDL_RenderGetViewport(sdl.renderer, &viewport);

    {
        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_vertex_layout_element vertex_layout[] = {
            {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_sdl_vertex, position)},
            {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_sdl_vertex, uv)},
            {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_sdl_vertex, col)},
            {NK_VERTEX_LAYOUT_END}};

        struct nk_convert_config config;
        NK_MEMSET(&config, 0, sizeof(config));
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(struct nk_sdl_vertex);
        config.vertex_alignment = NK_ALIGNOF(struct nk_sdl_vertex);
        config.null = dev->tex_null;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.shape_AA = NK_ANTI_ALIASING_ON;
        config.line_AA = NK_ANTI_ALIASING_ON;

        NK_MEMSET(&vbuf, 0, sizeof(vbuf));
        NK_MEMSET(&ebuf, 0, sizeof(ebuf));
        nk_buffer_init_default(&vbuf);
        nk_buffer_init_default(&ebuf);
        nk_convert(&sdl.ctx, &dev->cmds, &vbuf, &ebuf, &config);
    }

    /* iterate over and execute each draw command */
    offset = (const nk_draw_index*)nk_buffer_memory_const(&ebuf);
    nk_draw_foreach(cmd, &sdl.ctx, &dev->cmds)
    {
        if (!cmd->elem_count)
            continue;

        SDL_Rect r;
        r.x = (int)cmd->clip_rect.x;
        r.y = (int)cmd->clip_rect.y;
        r.w = (int)cmd->clip_rect.w;
        r.h = (int)cmd->clip_rect.h;
        SDL_RenderSetClipRect(sdl.renderer, &r);

        const struct nk_sdl_vertex* vertices =
            (const struct nk_sdl_vertex*)nk_buffer_memory_const(&vbuf);

        /* Simple triangle rendering - this is a basic implementation */
        for (unsigned int i = 0; i < cmd->elem_count; i += 3)
        {
            nk_draw_index i0 = offset[i + 0];
            nk_draw_index i1 = offset[i + 1];
            nk_draw_index i2 = offset[i + 2];

            const struct nk_sdl_vertex* v0 = &vertices[i0];
            const struct nk_sdl_vertex* v1 = &vertices[i1];
            const struct nk_sdl_vertex* v2 = &vertices[i2];

            SDL_Vertex sdl_vertices[3];
            sdl_vertices[0].position.x = v0->position[0];
            sdl_vertices[0].position.y = v0->position[1];
            sdl_vertices[0].color.r = v0->col[0];
            sdl_vertices[0].color.g = v0->col[1];
            sdl_vertices[0].color.b = v0->col[2];
            sdl_vertices[0].color.a = v0->col[3];
            sdl_vertices[0].tex_coord.x = v0->uv[0];
            sdl_vertices[0].tex_coord.y = v0->uv[1];

            sdl_vertices[1].position.x = v1->position[0];
            sdl_vertices[1].position.y = v1->position[1];
            sdl_vertices[1].color.r = v1->col[0];
            sdl_vertices[1].color.g = v1->col[1];
            sdl_vertices[1].color.b = v1->col[2];
            sdl_vertices[1].color.a = v1->col[3];
            sdl_vertices[1].tex_coord.x = v1->uv[0];
            sdl_vertices[1].tex_coord.y = v1->uv[1];

            sdl_vertices[2].position.x = v2->position[0];
            sdl_vertices[2].position.y = v2->position[1];
            sdl_vertices[2].color.r = v2->col[0];
            sdl_vertices[2].color.g = v2->col[1];
            sdl_vertices[2].color.b = v2->col[2];
            sdl_vertices[2].color.a = v2->col[3];
            sdl_vertices[2].tex_coord.x = v2->uv[0];
            sdl_vertices[2].tex_coord.y = v2->uv[1];

            SDL_RenderGeometry(sdl.renderer, (SDL_Texture*)cmd->texture.ptr, sdl_vertices, 3, NULL,
                               0);
        }
        offset += cmd->elem_count;
    }

    SDL_RenderSetClipRect(sdl.renderer, NULL);
    nk_clear(&sdl.ctx);
    nk_buffer_clear(&dev->cmds);
    nk_buffer_free(&vbuf);
    nk_buffer_free(&ebuf);
}

NK_API int nk_sdl_handle_event(SDL_Event* evt)
{
    struct nk_context* ctx = &sdl.ctx;
    if (evt->type == SDL_KEYUP || evt->type == SDL_KEYDOWN)
    {
        /* key events */
        int down = evt->type == SDL_KEYDOWN;
        const Uint8* state = SDL_GetKeyboardState(0);
        SDL_Keycode sym = evt->key.keysym.sym;
        if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT)
            nk_input_key(ctx, NK_KEY_SHIFT, down);
        else if (sym == SDLK_DELETE)
            nk_input_key(ctx, NK_KEY_DEL, down);
        else if (sym == SDLK_RETURN)
            nk_input_key(ctx, NK_KEY_ENTER, down);
        else if (sym == SDLK_TAB)
            nk_input_key(ctx, NK_KEY_TAB, down);
        else if (sym == SDLK_BACKSPACE)
            nk_input_key(ctx, NK_KEY_BACKSPACE, down);
        else if (sym == SDLK_HOME)
            nk_input_key(ctx, NK_KEY_TEXT_START, down);
        else if (sym == SDLK_END)
            nk_input_key(ctx, NK_KEY_TEXT_END, down);
        else if (sym == SDLK_PAGEDOWN)
            nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down);
        else if (sym == SDLK_PAGEUP)
            nk_input_key(ctx, NK_KEY_SCROLL_UP, down);
        else if (sym == SDLK_z)
            nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_r)
            nk_input_key(ctx, NK_KEY_TEXT_REDO, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_c)
            nk_input_key(ctx, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_v)
            nk_input_key(ctx, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_x)
            nk_input_key(ctx, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_b)
            nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_e)
            nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_UP)
            nk_input_key(ctx, NK_KEY_UP, down);
        else if (sym == SDLK_DOWN)
            nk_input_key(ctx, NK_KEY_DOWN, down);
        else if (sym == SDLK_LEFT)
        {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
            else
                nk_input_key(ctx, NK_KEY_LEFT, down);
        }
        else if (sym == SDLK_RIGHT)
        {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
            else
                nk_input_key(ctx, NK_KEY_RIGHT, down);
        }
        return 1;
    }
    else if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONUP)
    {
        /* mouse button */
        int down = evt->type == SDL_MOUSEBUTTONDOWN;
        const int x = evt->button.x, y = evt->button.y;
        if (evt->button.button == SDL_BUTTON_LEFT)
        {
            if (evt->button.clicks > 1)
                nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, down);
            nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
        }
        else if (evt->button.button == SDL_BUTTON_MIDDLE)
            nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
        else if (evt->button.button == SDL_BUTTON_RIGHT)
            nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
        return 1;
    }
    else if (evt->type == SDL_MOUSEMOTION)
    {
        /* mouse motion */
        if (ctx->input.mouse.grabbed)
        {
            int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
            nk_input_motion(ctx, x + evt->motion.xrel, y + evt->motion.yrel);
        }
        else
            nk_input_motion(ctx, evt->motion.x, evt->motion.y);
        return 1;
    }
    else if (evt->type == SDL_TEXTINPUT)
    {
        /* text input */
        nk_glyph glyph;
        memcpy(glyph, evt->text.text, NK_UTF_SIZE);
        nk_input_glyph(ctx, glyph);
        return 1;
    }
    else if (evt->type == SDL_MOUSEWHEEL)
    {
        /* mouse wheel */
        nk_input_scroll(ctx, nk_vec2((float)evt->wheel.x, (float)evt->wheel.y));
        return 1;
    }
    return 0;
}

NK_API void nk_sdl_shutdown(void)
{
    struct nk_sdl_device* dev = &sdl.ogl;
    nk_font_atlas_clear(&sdl.atlas);
    nk_free(&sdl.ctx);
    SDL_DestroyTexture(dev->font_tex);
    nk_buffer_free(&dev->cmds);
    memset(&sdl, 0, sizeof(sdl));
}

NK_API void nk_sdl_font_stash_begin(struct nk_font_atlas** atlas)
{
    nk_font_atlas_init_default(&sdl.atlas);
    nk_font_atlas_begin(&sdl.atlas);
    *atlas = &sdl.atlas;
}

NK_API void nk_sdl_font_stash_end(void)
{
    const void* image;
    int w, h;
    image = nk_font_atlas_bake(&sdl.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_sdl_device_upload_atlas(image, w, h);
    nk_font_atlas_end(&sdl.atlas, nk_handle_ptr(sdl.ogl.font_tex), &sdl.ogl.tex_null);
    if (sdl.atlas.default_font)
        nk_style_set_font(&sdl.ctx, &sdl.atlas.default_font->handle);
}

NK_API struct nk_context* nk_sdl_init(SDL_Window* win, SDL_Renderer* renderer)
{
    sdl.win = win;
    sdl.renderer = renderer;
    nk_init_default(&sdl.ctx, 0);
    nk_buffer_init_default(&sdl.ogl.cmds);
    return &sdl.ctx;
}

#endif /* NK_SDL_RENDERER_IMPLEMENTATION */
