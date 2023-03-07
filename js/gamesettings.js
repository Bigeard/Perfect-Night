const gameSettings = {
    edit: 0,
    maxScore: 3,
    // @TODO
    randomPosLoot: 0,
    defaultItem: 0,
    // selectItem: [0, 1, 2, 3, 4, 5, 6]
}

const changeSettings = (newSettings) => {
    for (let key in gameSettings) {
        const val = parseInt(newSettings[key]);
        if (val) gameSettings[key] = val;
    }
    gameSettings.edit = true;
    listGamepad.forEach(g => {
        // @TODO change value of the fields
        if (g.conn) g.conn.send(JSON.stringify(gameSettings));
    });
}
