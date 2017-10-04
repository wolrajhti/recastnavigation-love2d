#include "wrap_dtPathCorridor.h"

int wrap_dtPathCorridor_new(lua_State *L) {
	*static_cast<dtPathCorridor**>(lua_newuserdata(L, sizeof(dtPathCorridor*))) = new dtPathCorridor();

	if (luaL_newmetatable(L, LUA_META_WRAP_DTPATHCORRIDOR)) {
		static const luaL_Reg methods[] = {
			{"reset", reset},
			{"setCorridor", setCorridor},
			{"findCorners", wrap_dtPathCorridor_findCorners},
			{"__gc", wrap_dtPathCorridor_free},
			{nullptr, nullptr}
		};
		luaL_register(L, nullptr, methods);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}
	lua_setmetatable(L, -2);

	dtPathCorridor *pathCorridor = *static_cast<dtPathCorridor**>(luaL_checkudata(L, 1, LUA_META_WRAP_DTPATHCORRIDOR));

	pathCorridor->init(256);

	return 1;
}

static int reset(lua_State *L) {
	printf("\ndtPathCorridor:reset\n--------------------\n");
	dtPathCorridor *pathCorridor = *static_cast<dtPathCorridor**>(luaL_checkudata(L, 1, LUA_META_WRAP_DTPATHCORRIDOR));
	dtPolyRef *pStart = *static_cast<dtPolyRef**>(luaL_checkudata(L, 2, LUA_META_PATH));
	float *start = *static_cast<float**>(luaL_checkudata(L, 3, LUA_META_VECTOR3F));
	/* passing dtNavMesh here is for debug purpose only */
	dtNavMesh *navMesh = *static_cast<dtNavMesh**>(luaL_checkudata(L, 4, LUA_META_WRAP_DTNAVMESH));

	printf("start %.2f %.2f %.2f \n", start[0], start[1], start[2]);

	if (!navMesh->isValidPolyRef(*pStart)) {
		luaL_error(L, "Invalid dtPolyRef.");
	}

	pathCorridor->reset(*pStart, start);

	return 0;
}

static int setCorridor(lua_State *L) {
	printf("\ndtPathCorridor:setCorridor\n--------------------------\n");
	dtPathCorridor *pathCorridor = *static_cast<dtPathCorridor**>(luaL_checkudata(L, 1, LUA_META_WRAP_DTPATHCORRIDOR));
	float *end = *static_cast<float**>(luaL_checkudata(L, 2, LUA_META_VECTOR3F));
	dtPolyRef *path = *static_cast<dtPolyRef**>(luaL_checkudata(L, 3, LUA_META_PATH));
	int npoly = luaL_checkint(L, 4);

	/* passing dtNavMesh and dtNavMeshQuery here is for debug purpose only */
	dtNavMesh *navMesh = *static_cast<dtNavMesh**>(luaL_checkudata(L, 5, LUA_META_WRAP_DTNAVMESH));
	dtNavMeshQuery *navMeshQuery = *static_cast<dtNavMeshQuery**>(luaL_checkudata(L, 6, LUA_META_WRAP_DTNAVMESHQUERY));

	printf("end %.2f %.2f %.2f \n", end[0], end[1], end[2]);

	for (int i = 0; i < npoly; ++i) {
		if (!navMesh->isValidPolyRef(path[i])) {
			luaL_error(L, "Invalid dtPolyRef at path[%d]", i);
		}
	}

	pathCorridor->setCorridor(end, path, npoly);

	if (!pathCorridor->isValid(npoly, navMeshQuery, new dtQueryFilter())) {
		luaL_error(L, "Invalid corridor.\n");
	}

	return 0;
}

static int wrap_dtPathCorridor_findCorners(lua_State *L) {
	dtPathCorridor *pathCorridor = *static_cast<dtPathCorridor**>(luaL_checkudata(L, 1, LUA_META_WRAP_DTPATHCORRIDOR));
	dtNavMeshQuery *navMeshQuery = *static_cast<dtNavMeshQuery**>(luaL_checkudata(L, 2, LUA_META_WRAP_DTNAVMESHQUERY));

	float cornerVerts[256 * 3];
	unsigned char cornerFlags[256];
	dtPolyRef cornerPolys[256];
	int maxCorners = 256;

	int nCorners = pathCorridor->findCorners(
		cornerVerts,
		cornerFlags,
		cornerPolys,
		maxCorners,
		navMeshQuery,
		new dtQueryFilter()
	);

	// printf("nCorners = %i\n", nCorners);

	lua_newtable(L);

	const float *pos = pathCorridor->getPos();

	lua_pushnumber(L, 1 + 0 * 2);
	lua_pushnumber(L, pos[0]);
	lua_settable(L, -3);
	lua_pushnumber(L, 1 + 0 * 2 + 1);
	lua_pushnumber(L, pos[2]);
	lua_settable(L, -3);

	for (int i = 0; i < nCorners; ++i) {
		// printf("cornerVerts %f %f %f\n", cornerVerts[i * 3 + 0], cornerVerts[i * 3 + 1], cornerVerts[i * 3 + 2]);
		lua_pushnumber(L, 1 + (i + 1) * 2);
		lua_pushnumber(L, cornerVerts[i * 3 + 0]);
		lua_settable(L, -3);
		lua_pushnumber(L, 1 + (i + 1) * 2 + 1);
		lua_pushnumber(L, cornerVerts[i * 3 + 2]);
		lua_settable(L, -3);
	}

	return 1;
}

static int wrap_dtPathCorridor_free(lua_State *L) {
	// printf("wrap_dtPathCorridor_free\n");
	dtPathCorridor *pathCorridor = *static_cast<dtPathCorridor**>(luaL_checkudata(L, 1, LUA_META_WRAP_DTPATHCORRIDOR));
	delete pathCorridor;
	return 0;
}

extern "C" int luaopen_wrap_dtPathCorridor(lua_State *L) {
	lua_pushcfunction(L, wrap_dtPathCorridor_new);
	return 1;
}