zephyr_library()
zephyr_include_directories(include)

# ZMK core headers (zmk/hid.h, zmk/events/..., etc.)
if(DEFINED ZMK_BASE)
  zephyr_include_directories(${ZMK_BASE}/app/include)
else()
  # fallback: WEST_TOPDIR is usually /tmp/zmk-config
  zephyr_include_directories(${WEST_TOPDIR}/zmk/app/include)
endif()

zephyr_library_sources_ifdef(CONFIG_ZMK_MEJIRO
  src/behaviors/mejiro_core.c
  src/behaviors/mejiro_send_roman.c
  src/behaviors/behavior_mejiro.c
  src/behaviors/mejiro_tables.c

  src/behaviors/behavior_naginata.c
  src/naginata_func.c
  src/nglist.c
  src/nglistarray.c
)
