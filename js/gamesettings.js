const gameSettings = {
    edit: 0,
    maxScore: 3,
    defaultTypeItem: -1, // 0 = random / -1 = default item (none)
    defaultMaxTimerItem: -1, // 0 = no limit / -1 = default max timer
    activeLoot: 1,
    // @TODO
    // randomPosLoot: 0,
    // defaultItem: 0,
    // selectItem: [0, 1, 2, 3, 4, 5, 6]
}

const changeSettings = (newSettings) => {
    for (let key in gameSettings) {
        const val = parseInt(newSettings[key]);
        if (typeof val === "number") gameSettings[key] = val;
    }
    gameSettings.edit = true;
    listGamepad.forEach(g => {
        // @TODO change value of the fields
        if (g.conn) g.conn.send(JSON.stringify(gameSettings));
    });
}
