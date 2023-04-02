const wakeLockTrigger = async () => {
    if ("wakeLock" in navigator) {
        console.log("Screen Wake Lock API supported!");
    } else {
        console.log("Wake lock is not supported by this browser.");
    }
    try {
        const wakeLock = await navigator.wakeLock.request("screen");
    } catch (err) {
        // the wake lock request fails - usually system related, such being low on battery
        console.log(`${err.name}, ${err.message}`);
    }
}
wakeLockTrigger();