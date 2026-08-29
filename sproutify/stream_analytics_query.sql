-- Query for Azure Stream Analytics job query editor

--  Every 15 seconds, summarize moisture for the Power BI chart.
-- avg / min / max percent, and how many readings were dry, wet, or healthy.
SELECT
    System.Timestamp() AS windowEnd,
    AVG(soilMoisture) AS avgMoisture,
    MIN(soilMoisture) AS minMoisture,
    MAX(soilMoisture) AS maxMoisture,
    COUNT(*) AS sampleCount,
    SUM(CASE WHEN plantStatus = 'Underwatered' THEN 1 ELSE 0 END) AS underwaterEvents,
    SUM(CASE WHEN plantStatus = 'Overwatered' THEN 1 ELSE 0 END) AS overwaterEvents,
    SUM(CASE WHEN plantStatus = 'Severely Overwatered' THEN 1 ELSE 0 END) AS severeOverwaterEvents,
    SUM(CASE WHEN plantStatus = 'Healthy' THEN 1 ELSE 0 END) AS healthyEvents
INTO sproutifydashboard
FROM sproutifyinput
GROUP BY TumblingWindow(second, 15);