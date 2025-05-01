import sys
import datetime

def getYesterday():
    """
    Returns the date of yesterday in the format 'MM/DD/YYYY'.
    """
    yesterday = datetime.datetime.now() - datetime.timedelta(days=1)
    return yesterday

def updateLog(LogMessage, logFile='Log',):
    """ "
    Appends a log message to the specified log file and
    updates the last update time in the log file.
    Args:
        logFile (str): The path to the log file.
        LogMessage (str): The message to be logged.
    """

    with open(logFile, "r") as log:
        dateCreated = log.readline().split(": ")[1].strip()
        lastUpdate_str = log.readline().split(": ")[1].strip()
        log.seek(0)
        logData = log.read()

    dateCreated = datetime.datetime.strptime(dateCreated, "%m/%d/%Y")
    lastUpdate = datetime.datetime.strptime(lastUpdate_str, "%m/%d/%Y")

    daysSinceCreated = (datetime.datetime.now() - dateCreated).days
    daysSinceLastUpdate = (datetime.datetime.now() - lastUpdate).days


    print("Last updated: " + str(lastUpdate_str))
    logData = logData.replace(
        "Last updated: " + str(lastUpdate_str),
        "Last updated: " + datetime.datetime.now().strftime("%m/%d/%Y"),
    )

    if daysSinceLastUpdate == 0:
        logData += (
            "\t- " + str(datetime.datetime.now().time())[0:-7] + " " + LogMessage + "\n"
        )
    elif daysSinceLastUpdate == 1:
        logData += (
            f"\nDay {daysSinceCreated} -- {datetime.datetime.now().date()}\n"
            + "\t- "
            + str(datetime.datetime.now().time())[0:-7]
            + " "
            + LogMessage
            + "\n"
        )

    elif daysSinceLastUpdate > 1:
        logData += (
            f"\nDays {daysSinceCreated-daysSinceLastUpdate} to {daysSinceCreated-1} -- {getYesterday()}\n"
            + "\t- "
            + str(datetime.datetime.now().time())[0:-7]
            + " "
            + "Nothing was logged."
            + "\n"
        )
        logData += (
            f"\nDay {daysSinceCreated} -- {datetime.datetime.now().date()}\n"
            + "\t- "
            + str(datetime.datetime.now().time())[0:-7]
            + " "
            + LogMessage
            + "\n"
        )
    with open(logFile, "w") as log:
        log.write(logData)


if __name__ == "__main__":

    if len(sys.argv) <= 1:
        print("Usage: python updateLog.py <logMessage>")
        sys.exit(1)

    logMessage = str(sys.argv[1])
    updateLog( logMessage)
