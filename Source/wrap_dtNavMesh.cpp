#include "wrap_dtNavMesh.h"

/*static*/ int wrap_dtNavMesh_new(lua_State *L) {
	rcPolyMesh *polyMesh = *static_cast<rcPolyMesh**>(luaL_checkudata(L, 1, LUA_META_WRAP_RCPOLYMESH));

	*static_cast<dtNavMesh**>(lua_newuserdata(L, sizeof(dtNavMesh*))) = new dtNavMesh();

	if (luaL_newmetatable(L, LUA_META_WRAP_DTNAVMESH)) {
		static const luaL_Reg methods[] = {
			{"__gc", wrap_dtNavMesh_free},
			{nullptr, nullptr}
		};
		luaL_register(L, nullptr, methods);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}
	lua_setmetatable(L, -2);

	dtNavMesh *navMesh = *static_cast<dtNavMesh**>(luaL_checkudata(L, 2, LUA_META_WRAP_DTNAVMESH));

	dtNavMeshCreateParams params;

	memset(&params, 0, sizeof(params));

	params.verts = polyMesh->verts;
	params.vertCount = polyMesh->nverts;
	params.polys = polyMesh->polys;
	params.polyAreas = polyMesh->areas;
	params.polyFlags = polyMesh->flags;
	params.polyCount = polyMesh->npolys;
	params.nvp = polyMesh->nvp;
	params.walkableHeight = 1;
	params.walkableClimb = 0;
	params.cs = polyMesh->cs;
	params.ch = polyMesh->ch;
	params.buildBvTree = true;

	int offMeshConCount = 1;

	float offMeshConVerts[] = {150, 1, 75, 150, 1, 475};
	float offMeshConRads[] = {10};
	unsigned char offMeshConDirs[] = {DT_OFFMESH_CON_BIDIR};
	unsigned char offMeshConAreas[] = {5};
	unsigned short offMeshConFlags[] = {12};
	unsigned int offMeshConId[] = {666};

	params.offMeshConVerts = offMeshConVerts;
	params.offMeshConRad = offMeshConRads;
	params.offMeshConDir = offMeshConDirs;
	params.offMeshConAreas = offMeshConAreas;
	params.offMeshConFlags = offMeshConFlags;
	params.offMeshConUserID = offMeshConId;
	params.offMeshConCount = offMeshConCount;

	rcVcopy(params.bmin, polyMesh->bmin);
	rcVcopy(params.bmax, polyMesh->bmax);

	unsigned char* navData = 0;
	int navDataSize = 0;

	if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
	{
		luaL_error(L, "Could not build Detour navmesh.");
		return 0;
	}

	/* SOLO */
	navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);

	const dtNavMesh &mesh = *navMesh;

	for (int i = 0; i < navMesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = mesh.getTile(i);
		if (!tile->header) continue;
		printf("DEBUG tile->header->polyCount = %d\n", tile->header->polyCount);
		printf("DEBUG offMeshBase = %d\n", tile->header->offMeshBase);
		printf("DEBUG offMeshConCount = %d\n", tile->header->offMeshConCount);
	}

	/* TILED */
	// dtNavMeshParams *navMeshParams;
	// navMesh->init(navMeshParams);
	// navMesh->addTile(navData, navDataSize, DT_TILE_FREE_DATA, 0, 0);

	return 1;
}

static int wrap_dtNavMesh_free(lua_State *L) {
	printf("wrap_dtNavMesh_free\n");
	dtNavMesh *navMesh = *static_cast<dtNavMesh**>(luaL_checkudata(L, 1, LUA_META_WRAP_DTNAVMESH));
	delete navMesh;
	return 0;
}

extern "C" int luaopen_wrap_dtNavMesh(lua_State *L) {
	lua_pushcfunction(L, wrap_dtNavMesh_new);
	return 1;
}
