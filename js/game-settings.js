const gameSettings = {
    edit: 0,
    maxScore: 3,
    maxAmmunition: 3,
    defaultTypeItem: -1, // 0 = random / -1 = default item (none)
    defaultMaxTimerItem: -1, // 0 = no limit / -1 = default max timer
    activeLoot: 1,
    selectItemBonusAmmunition: 1,
    selectItemBonusLife: 1,
    selectItemBonusSpeed: 1,
    selectItemLaser: 1,
    selectItemMultiShoot: 1,
    selectItemRocket: 1,
    selectItemSword: 1,
    selectItemNothing: 1,
    selectItemWall: 1,
}

const changeSettings = (newSettings) => {
    for (let key in gameSettings) {
        const val = parseInt(newSettings[key]);
        if (Number.isFinite(val)) gameSettings[key] = val;
    }
    gameSettings.edit = true;
    listGamepad.forEach(g => {
        if (typeof sendPeerJson === "function") {
            sendPeerJson(g.conn, gameSettings);
        } else if (g.conn?.open) {
            g.conn.send(JSON.stringify(gameSettings));
        }
    });
}
