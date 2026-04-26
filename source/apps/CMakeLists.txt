target_sources(${PRODUCT} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/about_badge.c
        ${CMAKE_CURRENT_LIST_DIR}/asteroids.c
        ${CMAKE_CURRENT_LIST_DIR}/battlezone.c
        ${CMAKE_CURRENT_LIST_DIR}/clue.c
        # ${CMAKE_CURRENT_LIST_DIR}/cube.c
        # ${CMAKE_CURRENT_LIST_DIR}/etch-a-sketch.c
        ${CMAKE_CURRENT_LIST_DIR}/game_of_life.c
        # ${CMAKE_CURRENT_LIST_DIR}/ghost-detector.c
        ${CMAKE_CURRENT_LIST_DIR}/hacking_simulator.c
        ${CMAKE_CURRENT_LIST_DIR}/lunarlander.c
        # ${CMAKE_CURRENT_LIST_DIR}/magic-8-ball.c
        # ${CMAKE_CURRENT_LIST_DIR}/maze.c
        ${CMAKE_CURRENT_LIST_DIR}/microban.c
        # ${CMAKE_CURRENT_LIST_DIR}/pong.c
        ${CMAKE_CURRENT_LIST_DIR}/qc.c
        ${CMAKE_CURRENT_LIST_DIR}/smashout.c
        ${CMAKE_CURRENT_LIST_DIR}/simonSays.c
        # ${CMAKE_CURRENT_LIST_DIR}/tank-vs-tank.c
        ${CMAKE_CURRENT_LIST_DIR}/username.c
        # ${CMAKE_CURRENT_LIST_DIR}/gulag.c
        ${CMAKE_CURRENT_LIST_DIR}/rvasec_splash.c
        ${CMAKE_CURRENT_LIST_DIR}/slot_machine.c
        ${CMAKE_CURRENT_LIST_DIR}/test-screensavers.c
        ${CMAKE_CURRENT_LIST_DIR}/moon-patrol.c
        ${CMAKE_CURRENT_LIST_DIR}/badgey.c
        # ${CMAKE_CURRENT_LIST_DIR}/image-test.c
	${CMAKE_CURRENT_LIST_DIR}/2048.c
	${CMAKE_CURRENT_LIST_DIR}/puzzle-attack.c
	${CMAKE_CURRENT_LIST_DIR}/badgemon.c
	${CMAKE_CURRENT_LIST_DIR}/badgemon_unlock.c
        # ${CMAKE_CURRENT_LIST_DIR}/badge-app-template.c
	${CMAKE_CURRENT_LIST_DIR}/aagunner.c
	${CMAKE_CURRENT_LIST_DIR}/default_menu_app.c
	# ${CMAKE_CURRENT_LIST_DIR}/carousel_menu_app.c
	${CMAKE_CURRENT_LIST_DIR}/menu_2025.c
	${CMAKE_CURRENT_LIST_DIR}/screensaver_app.c
	${CMAKE_CURRENT_LIST_DIR}/drum_machine.c
	${CMAKE_CURRENT_LIST_DIR}/mixtape.c
        )
add_subdirectory(simonSays_assets)
add_subdirectory(clue_assets)
add_subdirectory(new_badge_monsters_assets)
add_subdirectory(new_badge_monsters)
add_subdirectory(mixtape-assets)
target_include_directories(${PRODUCT} PUBLIC .)

badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_birdo.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_bowser.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_crawler.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_ghosts.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_harpy.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_jason_voorhees.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_medusa_head.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_metall.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_moblin.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_mother_brain.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_nettler.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_odd_eye.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_shredder.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_slime.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_stay_puff.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_sinistar.png)

badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_beetlejuice.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_chet.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_chucky.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_drago.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_ed_rooney.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_freddy_krueger.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_gopher_caddyshack.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_gremlin.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_hans_gruber.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_jack_torrance.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_joker.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_khan.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_richard_vernon.png)
badge_png_asset_add(badgemon 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_skeltor.png)

badge_png_asset_build(badgemon ${PRODUCT})
