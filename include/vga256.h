#ifndef VGA256_H
#define VGA256_H

#include <stdint.h>

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Color;

// VGA 256-color palette definition
// Reference:
// https://hexdocs.pm/color_palette/ansi_color_codes.html

#define VGA_SYSTEM_BLACK {0, 0, 0}
#define VGA_SYSTEM_MAROON {128, 0, 0}
#define VGA_SYSTEM_OFFICE_GREEN {0, 128, 0}
#define VGA_SYSTEM_YELLOW_003 {128, 128, 0}
#define VGA_SYSTEM_BLUE_004 {0, 0, 128}
#define VGA_SYSTEM_PATRIARCH {128, 0, 128}
#define VGA_SYSTEM_CYAN_006 {0, 128, 128}
#define VGA_SYSTEM_ARGENT {192, 192, 192}
#define VGA_SYSTEM_GRAY {128, 128, 128}
#define VGA_SYSTEM_LIGHT_RED {255, 0, 0}
#define VGA_SYSTEM_ELECTRIC_GREEN {0, 255, 0}
#define VGA_SYSTEM_LIGHT_YELLOW_011 {255, 255, 0}
#define VGA_SYSTEM_BLUE {0, 0, 255}
#define VGA_SYSTEM_FUCHSIA {255, 0, 255}
#define VGA_SYSTEM_AQUA {0, 255, 255}
#define VGA_SYSTEM_LIGHT_WHITE {255, 255, 255}
#define VGA_BLACK {0, 0, 0}
#define VGA_FUZZY_WUZZY {0, 0, 95}
#define VGA_DARK_BLUE {0, 0, 135}
#define VGA_CARNATION_PINK {0, 0, 175}
#define VGA_MEDIUM_BLUE {0, 0, 215}
#define VGA_BLUE {0, 0, 255}
#define VGA_CAMARONE {0, 95, 0}
#define VGA_BANGLADESH_GREEN {0, 95, 95}
#define VGA_ORIENT {0, 95, 135}
#define VGA_ENDEAVOUR {0, 95, 175}
#define VGA_SCIENCE_BLUE {0, 95, 215}
#define VGA_BLUE_RIBBON {0, 95, 255}
#define VGA_AO {0, 135, 0}
#define VGA_DEEP_SEA {0, 135, 95}
#define VGA_TEAL {0, 135, 135}
#define VGA_DEEP_CERULEAN {0, 135, 175}
#define VGA_BLUE_COLA {0, 135, 215}
#define VGA_AZURE {0, 135, 255}
#define VGA_DARK_LIME_GREEN {0, 175, 0}
#define VGA_GO_GREEN {0, 175, 95}
#define VGA_DARK_CYAN {0, 175, 135}
#define VGA_BONDI_BLUE {0, 175, 175}
#define VGA_CERULEAN {0, 175, 215}
#define VGA_BLUE_BOLT {0, 175, 255}
#define VGA_STRONG_LIME_GREEN {0, 215, 0}
#define VGA_MALACHITE {0, 215, 95}
#define VGA_CARIBBEAN_GREEN_042 {0, 215, 135}
#define VGA_CARIBBEAN_GREEN {0, 215, 175}
#define VGA_DARK_TURQUOISE {0, 215, 215}
#define VGA_VIVID_SKY_BLUE {0, 215, 255}
#define VGA_ELECTRIC_GREEN {0, 255, 0}
#define VGA_SPRING_GREEN_047 {0, 255, 95}
#define VGA_GUPPIE_GREEN {0, 255, 135}
#define VGA_MEDIUM_SPRING_GREEN {0, 255, 175}
#define VGA_BRIGHT_TURQUOISE {0, 255, 215}
#define VGA_AQUA {0, 255, 255}
#define VGA_BLOOD_RED {95, 0, 0}
#define VGA_IMPERIAL_PURPLE {95, 0, 95}
#define VGA_METALLIC_VIOLET {95, 0, 135}
#define VGA_CHINESE_PURPLE {95, 0, 175}
#define VGA_ELECTRIC_VIOLET_056 {95, 0, 215}
#define VGA_ELECTRIC_INDIGO {95, 0, 255}
#define VGA_BRONZE_YELLOW {95, 95, 0}
#define VGA_SCORPION {95, 95, 95}
#define VGA_COMET {95, 95, 135}
#define VGA_DARK_MODERATE_BLUE {95, 95, 175}
#define VGA_INDIGO {95, 95, 215}
#define VGA_CORNFLOWER_BLUE {95, 95, 255}
#define VGA_AVOCADO {95, 135, 0}
#define VGA_GLADE_GREEN {95, 135, 95}
#define VGA_JUNIPER {95, 135, 135}
#define VGA_HIPPIE_BLUE {95, 135, 175}
#define VGA_HAVELOCK_BLUE {95, 135, 215}
#define VGA_BLUEBERRY {95, 135, 255}
#define VGA_DARK_GREEN {95, 175, 0}
#define VGA_DARK_MODERATE_LIME_GREEN {95, 175, 95}
#define VGA_POLISHED_PINE {95, 175, 135}
#define VGA_CRYSTAL_BLUE {95, 175, 175}
#define VGA_AQUA_PEARL {95, 175, 215}
#define VGA_BLUE_JEANS {95, 175, 255}
#define VGA_ALIEN_ARMPIT {95, 215, 0}
#define VGA_MODERATE_LIME_GREEN {95, 215, 95}
#define VGA_CARIBBEAN_GREEN_PEARL {95, 215, 135}
#define VGA_DOWNY {95, 215, 175}
#define VGA_MEDIUM_TURQUOISE {95, 215, 215}
#define VGA_MAYA_BLUE {95, 215, 255}
#define VGA_BRIGHT_GREEN {95, 255, 0}
#define VGA_LIGHT_LIME_GREEN {95, 255, 95}
#define VGA_VERY_LIGHT_MALACHITE_GREEN {95, 255, 135}
#define VGA_MEDIUM_AQUAMARINE {95, 255, 175}
#define VGA_AQUAMARINE_086 {95, 255, 215}
#define VGA_AQUAMARINE_087 {95, 255, 255}
#define VGA_DEEP_RED {135, 0, 0}
#define VGA_FRENCH_PLUM {135, 0, 95}
#define VGA_FRESH_EGGPLANT {135, 0, 135}
#define VGA_PURPLE {135, 0, 175}
#define VGA_FRENCH_VIOLET {135, 0, 215}
#define VGA_ELECTRIC_VIOLET {135, 0, 255}
#define VGA_BROWN {135, 95, 0}
#define VGA_COPPER_ROSE {135, 95, 95}
#define VGA_CHINESE_VIOLET {135, 95, 135}
#define VGA_DARK_MODERATE_VIOLET {135, 95, 175}
#define VGA_MEDIUM_PURPLE {135, 95, 215}
#define VGA_BLUEBERRY_099 {135, 95, 255}
#define VGA_DARK_YELLOW_OLIVE_TONE {135, 135, 0}
#define VGA_CLAY_CREEK {135, 135, 95}
#define VGA_TAUPE_GRAY {135, 135, 135}
#define VGA_COOL_GREY {135, 135, 175}
#define VGA_CHETWODE_BLUE {135, 135, 215}
#define VGA_VIOLETS_ARE_BLUE {135, 135, 255}
#define VGA_APPLE_GREEN {135, 175, 0}
#define VGA_ASPARAGUS {135, 175, 95}
#define VGA_BAY_LEAF {135, 175, 135}
#define VGA_DARK_GRAYISH_CYAN {135, 175, 175}
#define VGA_LIGHT_COBALT_BLUE {135, 175, 215}
#define VGA_FRENCH_SKY_BLUE {135, 175, 255}
#define VGA_PISTACHIO {135, 215, 0}
#define VGA_MANTIS {135, 215, 95}
#define VGA_PASTEL_GREEN {135, 215, 135}
#define VGA_PEARL_AQUA {135, 215, 175}
#define VGA_BERMUDA {135, 215, 215}
#define VGA_PALE_CYAN {135, 215, 255}
#define VGA_CHARTREUSE {135, 255, 0}
#define VGA_LIGHT_GREEN {135, 255, 95}
#define VGA_VERY_LIGHT_LIME_GREEN {135, 255, 135}
#define VGA_MINT_GREEN_121 {135, 255, 175}
#define VGA_AQUAMARINE {135, 255, 215}
#define VGA_ANAKIWA {135, 255, 255}
#define VGA_BRIGHT_RED {175, 0, 0}
#define VGA_DARK_PINK {175, 0, 95}
#define VGA_DARK_MAGENTA {175, 0, 135}
#define VGA_HELIOTROPE_MAGENTA {175, 0, 175}
#define VGA_VIVID_MULBERRY {175, 0, 215}
#define VGA_ELECTRIC_PURPLE {175, 0, 255}
#define VGA_DARK_ORANGE_BROWN_TONE {175, 95, 0}
#define VGA_DARK_MODERATE_RED {175, 95, 95}
#define VGA_DARK_MODERATE_PINK {175, 95, 135}
#define VGA_DARK_MODERATE_MAGENTA {175, 95, 175}
#define VGA_RICH_LILAC {175, 95, 215}
#define VGA_LAVENDER_INDIGO {175, 95, 255}
#define VGA_DARK_GOLDENROD {175, 135, 0}
#define VGA_BRONZE {175, 135, 95}
#define VGA_DARK_GRAYISH_RED {175, 135, 135}
#define VGA_BOUQUET {175, 135, 175}
#define VGA_LAVENDER {175, 135, 215}
#define VGA_BRIGHT_LAVENDER {175, 135, 255}
#define VGA_BUDDHA_GOLD {175, 175, 0}
#define VGA_DARK_MODERATE_YELLOW {175, 175, 95}
#define VGA_DARK_GRAYISH_YELLOW {175, 175, 135}
#define VGA_SILVER_FOIL {175, 175, 175}
#define VGA_GRAYISH_BLUE {175, 175, 215}
#define VGA_MAXIMUM_BLUE_PURPLE {175, 175, 255}
#define VGA_RIO_GRANDE {175, 215, 0}
#define VGA_CONIFER {175, 215, 95}
#define VGA_FEIJOA {175, 215, 135}
#define VGA_GRAYISH_LIME_GREEN {175, 215, 175}
#define VGA_CRYSTAL {175, 215, 215}
#define VGA_FRESH_AIR {175, 215, 255}
#define VGA_LIME {175, 255, 0}
#define VGA_GREEN_YELLOW {175, 255, 95}
#define VGA_MINT_GREEN {175, 255, 135}
#define VGA_MENTHOL {175, 255, 175}
#define VGA_AERO_BLUE {175, 255, 215}
#define VGA_CELESTE {175, 255, 255}
#define VGA_GUARDSMAN_RED {215, 0, 0}
#define VGA_RAZZMATAZZ {215, 0, 95}
#define VGA_MEXICAN_PINK {215, 0, 135}
#define VGA_HOLLYWOOD_CERISE_163 {215, 0, 175}
#define VGA_DEEP_MAGENTA {215, 0, 215}
#define VGA_PHLOX {215, 0, 255}
#define VGA_STRONG_ORANGE {215, 95, 0}
#define VGA_INDIAN_RED {215, 95, 95}
#define VGA_BLUSH {215, 95, 135}
#define VGA_HOPBUSH {215, 95, 175}
#define VGA_MODERATE_MAGENTA {215, 95, 215}
#define VGA_HELIOTROPE {215, 95, 255}
#define VGA_CHOCOLATE {215, 135, 0}
#define VGA_COPPERFIELD {215, 135, 95}
#define VGA_MY_PINK {215, 135, 135}
#define VGA_CAN_CAN {215, 135, 175}
#define VGA_DEEP_MAUVE {215, 135, 215}
#define VGA_BRIGHT_LILAC {215, 135, 255}
#define VGA_GOLDENROD {215, 175, 0}
#define VGA_EARTH_YELLOW {215, 175, 95}
#define VGA_SLIGHTLY_DESATURATED_ORANGE {215, 175, 135}
#define VGA_CLAM_SHELL {215, 175, 175}
#define VGA_GRAYISH_MAGENTA {215, 175, 215}
#define VGA_MAUVE {215, 175, 255}
#define VGA_CITRINE {215, 215, 0}
#define VGA_CHINESE_GREEN {215, 215, 95}
#define VGA_DECO {215, 215, 135}
#define VGA_GRAYISH_YELLOW {215, 215, 175}
#define VGA_LIGHT_SILVER {215, 215, 215}
#define VGA_FOG {215, 215, 255}
#define VGA_CHARTREUSE_YELLOW {215, 255, 0}
#define VGA_CANARY {215, 255, 95}
#define VGA_HONEYSUCKLE {215, 255, 135}
#define VGA_PALE_GREEN {215, 255, 175}
#define VGA_BEIGE {215, 255, 215}
#define VGA_LIGHT_CYAN {215, 255, 255}
#define VGA_LIGHT_RED {255, 0, 0}
#define VGA_VIVID_RASPBERRY {255, 0, 95}
#define VGA_BRIGHT_PINK {255, 0, 135}
#define VGA_FASHION_FUCHSIA {255, 0, 175}
#define VGA_PURE_MAGENTA {255, 0, 215}
#define VGA_FUCHSIA {255, 0, 255}
#define VGA_BLAZE_ORANGE {255, 95, 0}
#define VGA_BITTERSWEET {255, 95, 95}
#define VGA_STRAWBERRY {255, 95, 135}
#define VGA_HOT_PINK {255, 95, 175}
#define VGA_LIGHT_DEEP_PINK {255, 95, 215}
#define VGA_PINK_FLAMINGO {255, 95, 255}
#define VGA_AMERICAN_ORANGE {255, 135, 0}
#define VGA_CORAL {255, 135, 95}
#define VGA_TULIP {255, 135, 135}
#define VGA_PINK_SALMON {255, 135, 175}
#define VGA_LAVENDER_ROSE {255, 135, 215}
#define VGA_BLUSH_PINK {255, 135, 255}
#define VGA_CHINESE_YELLOW {255, 175, 0}
#define VGA_LIGHT_ORANGE {255, 175, 95}
#define VGA_HIT_PINK {255, 175, 135}
#define VGA_MELON {255, 175, 175}
#define VGA_COTTON_CANDY {255, 175, 215}
#define VGA_PALE_MAGENTA {255, 175, 255}
#define VGA_GOLD {255, 215, 0}
#define VGA_DANDELION {255, 215, 95}
#define VGA_GRANDIS {255, 215, 135}
#define VGA_CARAMEL {255, 215, 175}
#define VGA_COSMOS {255, 215, 215}
#define VGA_BUBBLE_GUM {255, 215, 255}
#define VGA_LIGHT_YELLOW_011 {255, 255, 0}
#define VGA_LASER_LEMON {255, 255, 95}
#define VGA_DOLLY {255, 255, 135}
#define VGA_CALAMANSI {255, 255, 175}
#define VGA_CREAM {255, 255, 215}
#define VGA_LIGHT_WHITE {255, 255, 255}
#define VGA_VAMPIRE_BLACK {8, 8, 8}
#define VGA_CHINESE_BLACK {18, 18, 18}
#define VGA_EERIE_BLACK {28, 28, 28}
#define VGA_RAISIN_BLACK {38, 38, 38}
#define VGA_DARK_CHARCOAL {48, 48, 48}
#define VGA_BLACK_OLIVE {58, 58, 58}
#define VGA_OUTER_SPACE {68, 68, 68}
#define VGA_DARK_LIVER {78, 78, 78}
#define VGA_DAVYS_GREY {88, 88, 88}
#define VGA_GRANITE_GRAY {98, 98, 98}
#define VGA_DIM_GRAY {108, 108, 108}
#define VGA_BOULDER {118, 118, 118}
#define VGA_GRAY {128, 128, 128}
#define VGA_PHILIPPINE_GRAY {138, 138, 138}
#define VGA_DUSTY_GRAY {148, 148, 148}
#define VGA_SPANISH_GRAY {158, 158, 158}
#define VGA_DARK_GRAY {168, 168, 168}
#define VGA_PHILIPPINE_SILVER {178, 178, 178}
#define VGA_SILVER {188, 188, 188}
#define VGA_SILVER_SAND {198, 198, 198}
#define VGA_AMERICAN_SILVER {208, 208, 208}
#define VGA_ALTO {218, 218, 218}
#define VGA_MERCURY {228, 228, 228}
#define VGA_BRIGHT_GRAY {238, 238, 238}

static const Color palette[256] = {
    VGA_SYSTEM_BLACK,                //   0
    VGA_SYSTEM_MAROON,               //   1
    VGA_SYSTEM_OFFICE_GREEN,         //   2
    VGA_SYSTEM_YELLOW_003,           //   3
    VGA_SYSTEM_BLUE_004,             //   4
    VGA_SYSTEM_PATRIARCH,            //   5
    VGA_SYSTEM_CYAN_006,             //   6
    VGA_SYSTEM_ARGENT,               //   7
    VGA_SYSTEM_GRAY,                 //   8
    VGA_SYSTEM_LIGHT_RED,            //   9
    VGA_SYSTEM_ELECTRIC_GREEN,       //  10
    VGA_SYSTEM_LIGHT_YELLOW_011,     //  11
    VGA_SYSTEM_BLUE,                 //  12
    VGA_SYSTEM_FUCHSIA,              //  13
    VGA_SYSTEM_AQUA,                 //  14
    VGA_SYSTEM_LIGHT_WHITE,          //  15
    VGA_BLACK,                       //  16
    VGA_FUZZY_WUZZY,                 //  17
    VGA_DARK_BLUE,                   //  18
    VGA_CARNATION_PINK,              //  19
    VGA_MEDIUM_BLUE,                 //  20
    VGA_BLUE,                        //  21
    VGA_CAMARONE,                    //  22
    VGA_BANGLADESH_GREEN,            //  23
    VGA_ORIENT,                      //  24
    VGA_ENDEAVOUR,                   //  25
    VGA_SCIENCE_BLUE,                //  26
    VGA_BLUE_RIBBON,                 //  27
    VGA_AO,                          //  28
    VGA_DEEP_SEA,                    //  29
    VGA_TEAL,                        //  30
    VGA_DEEP_CERULEAN,               //  31
    VGA_BLUE_COLA,                   //  32
    VGA_AZURE,                       //  33
    VGA_DARK_LIME_GREEN,             //  34
    VGA_GO_GREEN,                    //  35
    VGA_DARK_CYAN,                   //  36
    VGA_BONDI_BLUE,                  //  37
    VGA_CERULEAN,                    //  38
    VGA_BLUE_BOLT,                   //  39
    VGA_STRONG_LIME_GREEN,           //  40
    VGA_MALACHITE,                   //  41
    VGA_CARIBBEAN_GREEN_042,         //  42
    VGA_CARIBBEAN_GREEN,             //  43
    VGA_DARK_TURQUOISE,              //  44
    VGA_VIVID_SKY_BLUE,              //  45
    VGA_ELECTRIC_GREEN,              //  46
    VGA_SPRING_GREEN_047,            //  47
    VGA_GUPPIE_GREEN,                //  48
    VGA_MEDIUM_SPRING_GREEN,         //  49
    VGA_BRIGHT_TURQUOISE,            //  50
    VGA_AQUA,                        //  51
    VGA_BLOOD_RED,                   //  52
    VGA_IMPERIAL_PURPLE,             //  53
    VGA_METALLIC_VIOLET,             //  54
    VGA_CHINESE_PURPLE,              //  55
    VGA_ELECTRIC_VIOLET_056,         //  56
    VGA_ELECTRIC_INDIGO,             //  57
    VGA_BRONZE_YELLOW,               //  58
    VGA_SCORPION,                    //  59
    VGA_COMET,                       //  60
    VGA_DARK_MODERATE_BLUE,          //  61
    VGA_INDIGO,                      //  62
    VGA_CORNFLOWER_BLUE,             //  63
    VGA_AVOCADO,                     //  64
    VGA_GLADE_GREEN,                 //  65
    VGA_JUNIPER,                     //  66
    VGA_HIPPIE_BLUE,                 //  67
    VGA_HAVELOCK_BLUE,               //  68
    VGA_BLUEBERRY,                   //  69
    VGA_DARK_GREEN,                  //  70
    VGA_DARK_MODERATE_LIME_GREEN,    //  71
    VGA_POLISHED_PINE,               //  72
    VGA_CRYSTAL_BLUE,                //  73
    VGA_AQUA_PEARL,                  //  74
    VGA_BLUE_JEANS,                  //  75
    VGA_ALIEN_ARMPIT,                //  76
    VGA_MODERATE_LIME_GREEN,         //  77
    VGA_CARIBBEAN_GREEN_PEARL,       //  78
    VGA_DOWNY,                       //  79
    VGA_MEDIUM_TURQUOISE,            //  80
    VGA_MAYA_BLUE,                   //  81
    VGA_BRIGHT_GREEN,                //  82
    VGA_LIGHT_LIME_GREEN,            //  83
    VGA_VERY_LIGHT_MALACHITE_GREEN,  //  84
    VGA_MEDIUM_AQUAMARINE,           //  85
    VGA_AQUAMARINE_086,              //  86
    VGA_AQUAMARINE_087,              //  87
    VGA_DEEP_RED,                    //  88
    VGA_FRENCH_PLUM,                 //  89
    VGA_FRESH_EGGPLANT,              //  90
    VGA_PURPLE,                      //  91
    VGA_FRENCH_VIOLET,               //  92
    VGA_ELECTRIC_VIOLET,             //  93
    VGA_BROWN,                       //  94
    VGA_COPPER_ROSE,                 //  95
    VGA_CHINESE_VIOLET,              //  96
    VGA_DARK_MODERATE_VIOLET,        //  97
    VGA_MEDIUM_PURPLE,               //  98
    VGA_BLUEBERRY_099,               //  99
    VGA_DARK_YELLOW_OLIVE_TONE,      // 100
    VGA_CLAY_CREEK,                  // 101
    VGA_TAUPE_GRAY,                  // 102
    VGA_COOL_GREY,                   // 103
    VGA_CHETWODE_BLUE,               // 104
    VGA_VIOLETS_ARE_BLUE,            // 105
    VGA_APPLE_GREEN,                 // 106
    VGA_ASPARAGUS,                   // 107
    VGA_BAY_LEAF,                    // 108
    VGA_DARK_GRAYISH_CYAN,           // 109
    VGA_LIGHT_COBALT_BLUE,           // 110
    VGA_FRENCH_SKY_BLUE,             // 111
    VGA_PISTACHIO,                   // 112
    VGA_MANTIS,                      // 113
    VGA_PASTEL_GREEN,                // 114
    VGA_PEARL_AQUA,                  // 115
    VGA_BERMUDA,                     // 116
    VGA_PALE_CYAN,                   // 117
    VGA_CHARTREUSE,                  // 118
    VGA_LIGHT_GREEN,                 // 119
    VGA_VERY_LIGHT_LIME_GREEN,       // 120
    VGA_MINT_GREEN_121,              // 121
    VGA_AQUAMARINE,                  // 122
    VGA_ANAKIWA,                     // 123
    VGA_BRIGHT_RED,                  // 124
    VGA_DARK_PINK,                   // 125
    VGA_DARK_MAGENTA,                // 126
    VGA_HELIOTROPE_MAGENTA,          // 127
    VGA_VIVID_MULBERRY,              // 128
    VGA_ELECTRIC_PURPLE,             // 129
    VGA_DARK_ORANGE_BROWN_TONE,      // 130
    VGA_DARK_MODERATE_RED,           // 131
    VGA_DARK_MODERATE_PINK,          // 132
    VGA_DARK_MODERATE_MAGENTA,       // 133
    VGA_RICH_LILAC,                  // 134
    VGA_LAVENDER_INDIGO,             // 135
    VGA_DARK_GOLDENROD,              // 136
    VGA_BRONZE,                      // 137
    VGA_DARK_GRAYISH_RED,            // 138
    VGA_BOUQUET,                     // 139
    VGA_LAVENDER,                    // 140
    VGA_BRIGHT_LAVENDER,             // 141
    VGA_BUDDHA_GOLD,                 // 142
    VGA_DARK_MODERATE_YELLOW,        // 143
    VGA_DARK_GRAYISH_YELLOW,         // 144
    VGA_SILVER_FOIL,                 // 145
    VGA_GRAYISH_BLUE,                // 146
    VGA_MAXIMUM_BLUE_PURPLE,         // 147
    VGA_RIO_GRANDE,                  // 148
    VGA_CONIFER,                     // 149
    VGA_FEIJOA,                      // 150
    VGA_GRAYISH_LIME_GREEN,          // 151
    VGA_CRYSTAL,                     // 152
    VGA_FRESH_AIR,                   // 153
    VGA_LIME,                        // 154
    VGA_GREEN_YELLOW,                // 155
    VGA_MINT_GREEN,                  // 156
    VGA_MENTHOL,                     // 157
    VGA_AERO_BLUE,                   // 158
    VGA_CELESTE,                     // 159
    VGA_GUARDSMAN_RED,               // 160
    VGA_RAZZMATAZZ,                  // 161
    VGA_MEXICAN_PINK,                // 162
    VGA_HOLLYWOOD_CERISE_163,        // 163
    VGA_DEEP_MAGENTA,                // 164
    VGA_PHLOX,                       // 165
    VGA_STRONG_ORANGE,               // 166
    VGA_INDIAN_RED,                  // 167
    VGA_BLUSH,                       // 168
    VGA_HOPBUSH,                     // 169
    VGA_MODERATE_MAGENTA,            // 170
    VGA_HELIOTROPE,                  // 171
    VGA_CHOCOLATE,                   // 172
    VGA_COPPERFIELD,                 // 173
    VGA_MY_PINK,                     // 174
    VGA_CAN_CAN,                     // 175
    VGA_DEEP_MAUVE,                  // 176
    VGA_BRIGHT_LILAC,                // 177
    VGA_GOLDENROD,                   // 178
    VGA_EARTH_YELLOW,                // 179
    VGA_SLIGHTLY_DESATURATED_ORANGE, // 180
    VGA_CLAM_SHELL,                  // 181
    VGA_GRAYISH_MAGENTA,             // 182
    VGA_MAUVE,                       // 183
    VGA_CITRINE,                     // 184
    VGA_CHINESE_GREEN,               // 185
    VGA_DECO,                        // 186
    VGA_GRAYISH_YELLOW,              // 187
    VGA_LIGHT_SILVER,                // 188
    VGA_FOG,                         // 189
    VGA_CHARTREUSE_YELLOW,           // 190
    VGA_CANARY,                      // 191
    VGA_HONEYSUCKLE,                 // 192
    VGA_PALE_GREEN,                  // 193
    VGA_BEIGE,                       // 194
    VGA_LIGHT_CYAN,                  // 195
    VGA_LIGHT_RED,                   // 196
    VGA_VIVID_RASPBERRY,             // 197
    VGA_BRIGHT_PINK,                 // 198
    VGA_FASHION_FUCHSIA,             // 199
    VGA_PURE_MAGENTA,                // 200
    VGA_FUCHSIA,                     // 201
    VGA_BLAZE_ORANGE,                // 202
    VGA_BITTERSWEET,                 // 203
    VGA_STRAWBERRY,                  // 204
    VGA_HOT_PINK,                    // 205
    VGA_LIGHT_DEEP_PINK,             // 206
    VGA_PINK_FLAMINGO,               // 207
    VGA_AMERICAN_ORANGE,             // 208
    VGA_CORAL,                       // 209
    VGA_TULIP,                       // 210
    VGA_PINK_SALMON,                 // 211
    VGA_LAVENDER_ROSE,               // 212
    VGA_BLUSH_PINK,                  // 213
    VGA_CHINESE_YELLOW,              // 214
    VGA_LIGHT_ORANGE,                // 215
    VGA_HIT_PINK,                    // 216
    VGA_MELON,                       // 217
    VGA_COTTON_CANDY,                // 218
    VGA_PALE_MAGENTA,                // 219
    VGA_GOLD,                        // 220
    VGA_DANDELION,                   // 221
    VGA_GRANDIS,                     // 222
    VGA_CARAMEL,                     // 223
    VGA_COSMOS,                      // 224
    VGA_BUBBLE_GUM,                  // 225
    VGA_LIGHT_YELLOW_011,            // 226
    VGA_LASER_LEMON,                 // 227
    VGA_DOLLY,                       // 228
    VGA_CALAMANSI,                   // 229
    VGA_CREAM,                       // 230
    VGA_LIGHT_WHITE,                 // 231
    VGA_VAMPIRE_BLACK,               // 232
    VGA_CHINESE_BLACK,               // 233
    VGA_EERIE_BLACK,                 // 234
    VGA_RAISIN_BLACK,                // 235
    VGA_DARK_CHARCOAL,               // 236
    VGA_BLACK_OLIVE,                 // 237
    VGA_OUTER_SPACE,                 // 238
    VGA_DARK_LIVER,                  // 239
    VGA_DAVYS_GREY,                  // 240
    VGA_GRANITE_GRAY,                // 241
    VGA_DIM_GRAY,                    // 242
    VGA_BOULDER,                     // 243
    VGA_GRAY,                        // 244
    VGA_PHILIPPINE_GRAY,             // 245
    VGA_DUSTY_GRAY,                  // 246
    VGA_SPANISH_GRAY,                // 247
    VGA_DARK_GRAY,                   // 248
    VGA_PHILIPPINE_SILVER,           // 249
    VGA_SILVER,                      // 250
    VGA_SILVER_SAND,                 // 251
    VGA_AMERICAN_SILVER,             // 252
    VGA_ALTO,                        // 253
    VGA_MERCURY,                     // 254
    VGA_BRIGHT_GRAY,                 // 255
};

#endif // VGA256_H

