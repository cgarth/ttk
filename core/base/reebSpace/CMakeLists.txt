ttk_add_base_library(reebSpace
  SOURCES
    ReebSpace.cpp
  HEADERS
    ReebSpace.h
  DEPENDS
    fiberSurface
    geometry
    jacobiSet
    triangulation
  )
