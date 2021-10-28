#include "gamebutton.h"

void game::button::draw(const parts& using_parts) const
{
    struct part_and_coords {
        C2D_Image parts::* part;
        int x, y;
        float scaleX{1.0f}, scaleY{1.0f};
    };
    const part_and_coords pcs[] = {
        {&parts::tl, x, y},
        {&parts::bl, x, y + h - using_parts.bl.subtex->height},
        {&parts::tr, x + w - using_parts.tr.subtex->width, y },
        {&parts::br, x + w - using_parts.br.subtex->width, y + h - using_parts.br.subtex->height},

        {&parts::vl, x, y + using_parts.tl.subtex->height, 1.0f, h - (using_parts.tl.subtex->height + using_parts.bl.subtex->height)},
        {&parts::vr, x + w - using_parts.tr.subtex->width, y + using_parts.tr.subtex->height, 1.0f, h - (using_parts.tr.subtex->height + using_parts.br.subtex->height)},

        {&parts::ht, x + using_parts.tl.subtex->width, y, w - (using_parts.tl.subtex->width + using_parts.tr.subtex->width)},
        {&parts::hb, x + using_parts.bl.subtex->width, y + h - using_parts.bl.subtex->height, w - (using_parts.bl.subtex->width + using_parts.br.subtex->width)},
    };
    for(const auto& pc : pcs)
    {
        C2D_DrawImageAt(using_parts.*(pc.part), pc.x, pc.y, depth, nullptr, pc.scaleX, pc.scaleY);
    }
    C2D_DrawRectSolid(
        x + using_parts.tl.subtex->width, y + using_parts.tl.subtex->height,
        depth,
        w - (using_parts.tl.subtex->width + using_parts.tr.subtex->width), h - (using_parts.tl.subtex->height + using_parts.bl.subtex->height),
        u32(using_parts.inner_color)
    );
}
