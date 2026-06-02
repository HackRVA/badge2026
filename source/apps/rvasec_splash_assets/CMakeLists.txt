target_sources(${PRODUCT} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/rib-intro.c
    ${CMAKE_CURRENT_LIST_DIR}/rib-theme.c
    ${CMAKE_CURRENT_LIST_DIR}/sponsor_logo.c
    ${CMAKE_CURRENT_LIST_DIR}/rvasec15_assets.c)

target_add_png_badge_asset(${PRODUCT} 8 ${CMAKE_CURRENT_LIST_DIR}/rvasec15_glass.png EXTRA_FLAGS --nostatic)
target_add_png_badge_asset(${PRODUCT} 8 ${CMAKE_CURRENT_LIST_DIR}/rvasec15_logo.png EXTRA_FLAGS --nostatic)
target_add_png_badge_asset(${PRODUCT} 8 ${CMAKE_CURRENT_LIST_DIR}/rvasec15_raven.png EXTRA_FLAGS --nostatic)
