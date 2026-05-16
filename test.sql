g
   ============================================================ */
DECLARE @StatsSql NVARCHAR(MAX) = N'
    -- Get baseline transaction counts
    SELECT
        COUNT(*) AS Total_Txns,
        SUM(CASE WHEN ' + QUOTENAME(@ColDirection) + N' = @ValDebit THEN 1 ELSE 0 END) AS Total_Debits,
        SUM(CASE WHEN ' + QUOTENAME(@ColDirection) + N' = @ValCredit THEN 1 ELSE 0 END) AS Total_Credits
    INTO ##SourceTotals
    FROM ' + @TableName + N';

    OPEN debit_cursor;
        END

        FETCH NEXT FROM debit_cursor INTO @CurrentDebitRowID, @CurrentCust, @CurrentAcc, @CurrentAmt, @CurrentDate;
    FETCH NEXT FROM debit_cursor INTO @CurrentDebitRowID, @CurrentCust, @CurrentAcc, @CurrentAmt, @CurrentDate;

    -- Get baseline customer account counts
    SELECT Customer_Number, COUNT(DISTINCT Account_Number) AS Acct_Count
    INTO ##CustomerAccounts
    FROM (
        SELECT 
            CAST(' + QUOTENAME(@ColCustomerNumber) + N' AS NVARCHAR(128)) AS Customer_Number,
            CAST(' + QUOTENAME(@ColAccountNumber)  + N' AS NVARCHAR(128)) AS Account_Number
        FROM ' + @TableName + N'
    ) AS Src
    GROUP BY Customer_Number;
-- 1A. Source Data Configuration
DECLARE @TableName            NVARCHAR(256) = N'Temp_test_data';
DECLARE @ColDate              NVARCHAR(128) = N'Transaction_Date';
DECLARE @TblAllFlagged        NVARCHAR(256) = N'dbo.Temp_all_transactions_with_flag';
DECLARE @TblStats             NVARCHAR(256) = N'dbo.Temp_Stats';

;WITH BaseAgg AS (
    SELECT
        COUNT(*)                                         AS Total_Txns,
        SUM(CASE WHEN Direction  = 'D' THEN 1 ELSE 0 END) AS Total_Debits,
        SUM(CASE WHEN Direction  = 'C' THEN 1 ELSE 0 END) AS Total_Credits,
        COUNT(DISTINCT Customer_Number)                  AS Distinct_Customers,
        COUNT(DISTINCT Account_Number)                   AS Distinct_Accounts,
        SUM(CASE WHEN Intra_Flag = 1   THEN 1 ELSE 0 END) AS Flagged_Count,
        SUM(CASE WHEN Intra_Flag = 0   THEN 1 ELSE 0 END) AS Unflagged_Count
    FROM dbo.Temp_all_transactions_with_flag
),
BucketSummary AS (
    SELECT
        COUNT(*)                                           AS Total_Customers,
        SUM(CASE WHEN Acct_Count = 1  THEN 1 ELSE 0 END)   AS Cnt_1,
        SUM(CASE WHEN Acct_Count = 2  THEN 1 ELSE 0 END)   AS Cnt_2,
        SUM(CASE WHEN Acct_Count = 3  THEN 1 ELSE 0 END)   AS Cnt_3,
        SUM(CASE WHEN Acct_Count = 4  THEN 1 ELSE 0 END)   AS Cnt_4,
        SUM(CASE WHEN Acct_Count = 5  THEN 1 ELSE 0 END)   AS Cnt_5,
        SUM(CASE WHEN Acct_Count >= 6 THEN 1 ELSE 0 END)   AS Cnt_6Plus,
        SUM(CASE WHEN Acct_Count > 1  THEN 1 ELSE 0 END)   AS Multi_Acct_Customers,
        MAX(Acct_Count)                                    AS Max_Accts
    FROM ##CustomerAccounts
),
MatchedStats AS (
    SELECT
        COUNT(*)                                                    AS Total_Pairs,
        SUM(CASE WHEN Match_Type = 'Exact'     THEN 1 ELSE 0 END) AS Exact_Pairs,
        SUM(CASE WHEN Match_Type = 'Threshold' THEN 1 ELSE 0 END) AS Threshold_Pairs
    FROM dbo.Temp_matched_pairs
)
INSERT INTO dbo.Temp_Stats (Stat_Category, Stat_Label, Stat_Value)
-- 1. Transaction Overview
SELECT '1. Transaction Overview', 'Total transactions (all customers)',        CAST(S.Total_Txns    AS NVARCHAR(100)) FROM ##SourceTotals S UNION ALL
SELECT '1. Transaction Overview', 'Total debit transactions (all customers)',  CAST(S.Total_Debits  AS NVARCHAR(100)) FROM ##SourceTotals S UNION ALL
SELECT '1. Transaction Overview', 'Total credit transactions (all customers)', CAST(S.Total_Credits AS NVARCHAR(100)) FROM ##SourceTotals S UNION ALL
-- 2. Customer & Account Distribution
SELECT '2. Customer & Account Distribution', 'Customers with 1 account',   CAST(BS.Cnt_1 AS NVARCHAR(50)) + ' (' + CAST(ROUND(100.0 * BS.Cnt_1 / NULLIF(BS.Total_Customers, 0), 2) AS NVARCHAR(50)) + '%)' FROM BucketSummary BS UNION ALL
SELECT '2. Customer & Account Distribution', 'Customers with 2 accounts',  CAST(BS.Cnt_2 AS NVARCHAR(50)) + ' (' + CAST(ROUND(100.0 * BS.Cnt_2 / NULLIF(BS.Total_Customers, 0), 2) AS NVARCHAR(50)) + '%)' FROM BucketSummary BS UNION ALL
SELECT '2. Customer & Account Distribution', 'Customers with 3 accounts',  CAST(BS.Cnt_3 AS NVARCHAR(50)) + ' (' + CAST(ROUND(100.0 * BS.Cnt_3 / NULLIF(BS.Total_Customers, 0), 2) AS NVARCHAR(50)) + '%)' FROM BucketSummary BS UNION ALL
SELECT '2. Customer & Account Distribution', 'Customers with 4 accounts',  CAST(BS.Cnt_4 AS NVARCHAR(50)) + ' (' + CAST(ROUND(100.0 * BS.Cnt_4 / NULLIF(BS.Total_Customers, 0), 2) AS NVARCHAR(50)) + '%)' FROM BucketSummary BS UNION ALL
SELECT '2. Customer & Account Distribution', 'Customers with 5 accounts',  CAST(BS.Cnt_5 AS NVARCHAR(50)) + ' (' + CAST(ROUND(100.0 * BS.Cnt_5 / NULLIF(BS.Total_Customers, 0), 2) AS NVARCHAR(50)) + '%)' FROM BucketSummary BS UNION ALL
SELECT '2. Customer & Account Distribution', 'Customers with 6+ accounts', CAST(BS.Cnt_6Plus AS NVARCHAR(50)) + ' (' + CAST(ROUND(100.0 * BS.Cnt_6Plus / NULLIF(BS.Total_Customers, 0), 2) AS NVARCHAR(50)) + '%)' FROM BucketSummary BS UNION ALL
-- 3. Multi-Account Summary
SELECT '3. Multi-Account Summary', 'Total transactions',                  CAST(B.Total_Txns           AS NVARCHAR(100)) FROM BaseAgg B UNION ALL
SELECT '3. Multi-Account Summary', 'Total distinct customers',            CAST(B.Distinct_Customers   AS NVARCHAR(100)) FROM BaseAgg B UNION ALL
SELECT '3. Multi-Account Summary', 'Total multi-account customers',       CAST(BS.Multi_Acct_Customers AS NVARCHAR(100)) FROM BucketSummary BS UNION ALL
-- 4. Matching Results
SELECT '4. Matching Results', 'Total matched pairs',                      CAST(MS.Total_Pairs         AS NVARCHAR(100)) FROM MatchedStats MS UNION ALL
SELECT '4. Matching Results', 'Exact match pairs',                        CAST(MS.Exact_Pairs         AS NVARCHAR(100)) FROM MatchedStats MS UNION ALL
SELECT '4. Matching Results', 'Threshold match pairs',                    CAST(MS.Threshold_Pairs     AS NVARCHAR(100)) FROM MatchedStats MS UNION ALL
SELECT '4. Matching Results', 'Intra-flagged transactions (Intra_Flag=1)',CAST(B.Flagged_Count AS NVARCHAR(50)) + ' (' + CAST(CAST(ROUND(100.0 * B.Flagged_Count / NULLIF(B.Total_Txns, 0), 2) AS DECIMAL(10,2)) AS NVARCHAR(20)) + '%)' FROM BaseAgg B UNION ALL
SELECT '4. Matching Results', 'Non-flagged transactions (Intra_Flag=0)',  CAST(B.Unflagged_Count AS NVARCHAR(50)) + ' (' + CAST(CAST(ROUND(100.0 * B.Unflagged_Count / NULLIF(B.Total_Txns, 0), 2) AS DECIMAL(10,2)) AS NVARCHAR(20)) + '%)' FROM BaseAgg B UNION ALL
-- 5. Run Configuration
SELECT '5. Run Configuration', 'Source table',                            @TableName UNION ALL
SELECT '5. Run Configuration', 'Day threshold used',                      CAST(@DayThreshold    AS NVARCHAR(100)) UNION ALL
SELECT '5. Run Configuration', 'Amount threshold used',                   CAST(@AmountThreshold AS NVARCHAR(100));

    -- 4B. Open a high-performance cursor over UNMATCHED debits only
    DECLARE debit_cursor CURSOR LOCAL FAST_FORWARD FOR
    SELECT Transaction_Row_ID, Customer_Number, Account_Number, Amount, Txn_Date
    FROM #Debits
    WHERE Matched_Group_ID IS NULL;

        -- 4D. Find the absolute closest available credit
        SELECT TOP (1) @FoundCreditRowID = Transaction_Row_ID
        FROM #Credits
        WHERE Matched_Group_ID IS NULL -- Ensures credit is not already taken
          AND Customer_Number = @CurrentCust
          AND Account_Number <> @CurrentAcc
          -- Preserving your original logic: Credit must happen on or after Debit date
          AND Txn_Date >= @CurrentDate 
          AND Txn_Date <= DATEADD(DAY, @DayThreshold, @CurrentDate)
          AND ABS(Amount - @CurrentAmt) <= @AmountThreshold
        ORDER BY 
          ABS(DATEDIFF(DAY, Txn_Date, @CurrentDate)) ASC, -- Prioritize closest date
          ABS(Amount - @CurrentAmt) ASC;/* ============================================================
   STEP 1: CONFIGURATION & ENVIRONMENT SETUP
   ============================================================ */
SET NOCOUNT ON;                  -- Tie-breaker: closest amount

        -- 4E. If a match is found, instantly lock both rows
        IF @FoundCreditRowID IS NOT NULL
        BEGIN
            SET @NewMatchID = NEWID();

-- 1B. Threshold Configuration (Only non-zero limits applied later)
DECLARE @DayThreshold         INT           = 0;
DECLARE @ColAccountNumber     NVARCHAR(128) = N'Masked_Account_Number';
DECLARE @ColCustomerNumber    NVARCHAR(128) = N'Masked_Customer_Number';
DECLARE @ColAmount            NVARCHAR(128) = N'Transaction_Amount';
DECLARE @ColDirection         NVARCHAR(128) = N'Debit_Credit_Indicator';

EXEC sp_executesql @StagingSql,
    N'@ValDebit NVARCHAR(64), @ValCredit NVARCHAR(64)',
    @ValDebit, @ValCredit;

-- 2C. Apply Performance Indexes
-- Clustered index on state tracker for the fuzzy loop
CREATE CLUSTERED INDEX CIX_Debits_MatchState ON #Debits (Matched_Group_ID);
DECLARE @AmountThreshold      DECIMAL(18,4) = 0;

-- 2B. Split into #Debits and #Credits with Sequence and State Tracking
SELECT
    Transaction_Row_ID,
    Customer_Number,
    Account_Number,
    Amount,
    Txn_Date,
    Direction,
    ROW_NUMBER() OVER (
        PARTITION BY Customer_Number, Txn_Date, Amount
        ORDER BY Transaction_Row_ID
    ) AS Duplicate_Seq,
    CAST(NULL AS UNIQUEIDENTIFIER) AS Matched_Group_ID
INTO #Debits
FROM ##StagingData
WHERE Direction = @ValDebit;
    DEALLOCATE debit_cursor;
IF OBJECT_ID('dbo.Temp_Stats', 'U') IS NOT NULL DROP TABLE dbo.Temp_Stats;

INSERT INTO @ExactMatches (Debit_Row_ID, Credit_Row_ID, Shared_Match_ID)
SELECT
    D.Transaction_Row_ID,
    C.Transaction_Row_ID,
    NEWID() -- Generates a unique GUID for this specific pair
FROM #Debits D
INNER JOIN #Credits C
    ON  D.Customer_Number = C.Customer_Number
    AND D.Txn_Date = C.Txn_Date
    AND D.Amount = C.Amount
    AND D.Duplicate_Seq = C.Duplicate_Seq -- The zipper: enforces 1-to-1 for duplicates
    AND D.Account_Number <> C.Account_Number;

-- 3C. Update state on #Credits
UPDATE C
SET C.Matched_Group_ID = E.Shared_Match_ID
FROM #Credits C
INNER JOIN @ExactMatches E 
    ON C.Transaction_Row_ID = E.Credit_Row_ID;


/* ============================================================
   STEP 6: CLEANUP
   ============================================================ */
IF OBJECT_ID('tempdb..##StagingData') IS NOT NULL DROP TABLE ##StagingData;
IF OBJECT_ID('tempdb..#Debits') IS NOT NULL DROP TABLE #Debits;
IF OBJECT_ID('tempdb..#Debits') IS NOT NULL DROP TABLE #Debits;
IF OBJECT_ID('tempdb..#Credits') IS NOT NULL DROP TABLE #Credits;
IF OBJECT_ID('tempdb..#Credits') IS NOT NULL DROP TABLE #Credits;
IF OBJECT_ID('tempdb..##SourceTotals') IS NOT NULL DROP TABLE ##SourceTotals;
IF OBJECT_ID('tempdb..##SourceTotals') IS NOT NULL DROP TABLE ##SourceTotals;
IF OBJECT_ID('tempdb..##CustomerAccounts') IS NOT NULL DROP TABLE ##CustomerAccounts;

/* ============================================================
   CLEANUP: DROP EXISTING TABLES FROM PRIOR RUNS
   ============================================================ */
IF OBJECT_ID('dbo.Temp_matched_pairs', 'U') IS NOT NULL DROP TABLE dbo.Temp_matched_pairs;
IF OBJECT_ID('dbo.Temp_merged_data', 'U') IS NOT NULL DROP TABLE dbo.Temp_merged_data;
IF OBJECT_ID('dbo.Temp_all_transactions_with_flag', 'U') IS NOT NULL DROP TABLE dbo.Temp_all_transactions_with_flag;
IF OBJECT_ID('tempdb..##CustomerAccounts') IS NOT NULL DROP TABLE ##CustomerAccounts;

/* ============================================================
   BASELINE STATISTICS: Capture total population before filtering
   ============================================================ */
DECLARE @StatsSql NVARCHAR(MAX) = N'
    -- Get baseline transaction counts
    SELECT
        COUNT(*) AS Total_Txns,
        SUM(CASE WHEN ' + QUOTENAME(@ColDirection) + N' = @ValDebit THEN 1 ELSE 0 END) AS Total_Debits,
        SUM(CASE WHEN ' + QUOTENAME(@ColDirection) + N' = @ValCredit THEN 1 ELSE 0 END) AS Total_Credits
    INTO ##SourceTotals
    FROM ' + @TableName + N';

    OPEN debit_cursor;
        END

        FETCH NEXT FROM debit_cursor INTO @CurrentDebitRowID, @CurrentCust, @CurrentAcc, @CurrentAmt, @CurrentDate;
    FETCH NEXT FROM debit_cursor INTO @CurrentDebitRowID, @CurrentCust, @CurrentAcc, @CurrentAmt, @CurrentDate;

    -- Get baseline customer account counts
    SELECT Customer_Number, COUNT(DISTINCT Account_Number) AS Acct_Count
    INTO ##CustomerAccounts
    FROM (
        SELECT 
            CAST(' + QUOTENAME(@ColCustomerNumber) + N' AS NVARCHAR(128)) AS Customer_Number,
            CAST(' + QUOTENAME(@ColAccountNumber)  + N' AS NVARCHAR(128)) AS Account_Number
        FROM ' + @TableName + N'
    ) AS Src
    GROUP BY Customer_Number;
-- 1A. Source Data Configuration
DECLARE @TableName            NVARCHAR(256) = N'Temp_test_data';
DECLARE @ColDate              NVARCHAR(128) = N'Transaction_Date';
DECLARE @TblAllFlagged        NVARCHAR(256) = N'dbo.Temp_all_transactions_with_flag';
DECLARE @TblStats             NVARCHAR(256) = N'dbo.Temp_Stats';

;WITH BaseAgg AS (
    SELECT
        COUNT(*)                                         AS Total_Txns,
        SUM(CASE WHEN Direction  = 'D' THEN 1 ELSE 0 END) AS Total_Debits,
        SUM(CASE WHEN Direction  = 'C' THEN 1 ELSE 0 END) AS Total_Credits,
        COUNT(DISTINCT Customer_Number)                  AS Distinct_Customers,
        COUNT(DISTINCT Account_Number)                   AS Distinct_Accounts,
        SUM(CASE WHEN Intra_Flag = 1   THEN 1 ELSE 0 END) AS Flagged_Count,
        SUM(CASE WHEN Intra_Flag = 0   THEN 1 ELSE 0 END) AS Unflagged_Count
    FROM dbo.Temp_all_transactions_with_flag
),
BucketSummary AS (
    SELECT
        COUNT(*)                                           AS Total_Customers,
        SUM(CASE WHEN Acct_Count = 1  THEN 1 ELSE 0 END)   AS Cnt_1,
        SUM(CASE WHEN Acct_Count = 2  THEN 1 ELSE 0 END)   AS Cnt_2,
        SUM(CASE WHEN Acct_Count = 3  THEN 1 ELSE 0 END)   AS Cnt_3,
        SUM(CASE WHEN Acct_Count = 4  THEN 1 ELSE 0 END)   AS Cnt_4,
        SUM(CASE WHEN Acct_Count = 5  THEN 1 ELSE 0 END)   AS Cnt_5,
        SUM(CASE WHEN Acct_Count >= 6 THEN 1 ELSE 0 END)   AS Cnt_6Plus,
        SUM(CASE WHEN Acct_Count > 1  THEN 1 ELSE 0 END)   AS Multi_Acct_Customers,
        MAX(Acct_Count)                                    AS Max_Accts
    FROM ##CustomerAccounts
),
MatchedStats AS (
    SELECT
        COUNT(*)                                                    AS Total_Pairs,
        SUM(CASE WHEN Match_Type = 'Exact'     THEN 1 ELSE 0 END) AS Exact_Pairs,
        SUM(CASE WHEN Match_Type = 'Threshold' THEN 1 ELSE 0 END) AS Threshold_Pairs
    FROM dbo.Temp_matched_pairs
)
INSERT INTO dbo.Temp_Stats (Stat_Category, Stat_Label, Stat_Value)
-- 1. Transaction Overview
SELECT '1. Transaction Overview', 'Total transactions (all customers)',        CAST(S.Total_Txns    AS NVARCHAR(100)) FROM ##SourceTotals S UNION ALL
SELECT '1. Transaction Overview', 'Total debit transactions (all customers)',  CAST(S.Total_Debits  AS NVARCHAR(100)) FROM ##SourceTotals S UNION ALL
SELECT '1. Transaction Overview', 'Total credit transactions (all customers)', CAST(S.Total_Credits AS NVARCHAR(100)) FROM ##SourceTotals S UNION ALL
-- 2. Customer & Account Distribution
SELECT '2. Customer & Account Distribution', 'Customers with 1 account',   CAST(BS.Cnt_1 AS NVARCHAR(50)) + ' (' + CAST(ROUND(100.0 * BS.Cnt_1 / NULLIF(BS.Total_Customers, 0), 2) AS NVARCHAR(50)) + '%)' FROM BucketSummary BS UNION ALL
SELECT '2. Customer & Account Distribution', 'Customers with 2 accounts',  CAST(BS.Cnt_2 AS NVARCHAR(50)) + ' (' + CAST(ROUND(100.0 * BS.Cnt_2 / NULLIF(BS.Total_Customers, 0), 2) AS NVARCHAR(50)) + '%)' FROM BucketSummary BS UNION ALL
SELECT '2. Customer & Account Distribution', 'Customers with 3 accounts',  CAST(BS.Cnt_3 AS NVARCHAR(50)) + ' (' + CAST(ROUND(100.0 * BS.Cnt_3 / NULLIF(BS.Total_Customers, 0), 2) AS NVARCHAR(50)) + '%)' FROM BucketSummary BS UNION ALL
SELECT '2. Customer & Account Distribution', 'Customers with 4 accounts',  CAST(BS.Cnt_4 AS NVARCHAR(50)) + ' (' + CAST(ROUND(100.0 * BS.Cnt_4 / NULLIF(BS.Total_Customers, 0), 2) AS NVARCHAR(50)) + '%)' FROM BucketSummary BS UNION ALL
SELECT '2. Customer & Account Distribution', 'Customers with 5 accounts',  CAST(BS.Cnt_5 AS NVARCHAR(50)) + ' (' + CAST(ROUND(100.0 * BS.Cnt_5 / NULLIF(BS.Total_Customers, 0), 2) AS NVARCHAR(50)) + '%)' FROM BucketSummary BS UNION ALL
SELECT '2. Customer & Account Distribution', 'Customers with 6+ accounts', CAST(BS.Cnt_6Plus AS NVARCHAR(50)) + ' (' + CAST(ROUND(100.0 * BS.Cnt_6Plus / NULLIF(BS.Total_Customers, 0), 2) AS NVARCHAR(50)) + '%)' FROM BucketSummary BS UNION ALL
-- 3. Multi-Account Summary
SELECT '3. Multi-Account Summary', 'Total transactions',                  CAST(B.Total_Txns           AS NVARCHAR(100)) FROM BaseAgg B UNION ALL
SELECT '3. Multi-Account Summary', 'Total distinct customers',            CAST(B.Distinct_Customers   AS NVARCHAR(100)) FROM BaseAgg B UNION ALL
SELECT '3. Multi-Account Summary', 'Total multi-account customers',       CAST(BS.Multi_Acct_Customers AS NVARCHAR(100)) FROM BucketSummary BS UNION ALL
-- 4. Matching Results
SELECT '4. Matching Results', 'Total matched pairs',                      CAST(MS.Total_Pairs         AS NVARCHAR(100)) FROM MatchedStats MS UNION ALL
SELECT '4. Matching Results', 'Exact match pairs',                        CAST(MS.Exact_Pairs         AS NVARCHAR(100)) FROM MatchedStats MS UNION ALL
SELECT '4. Matching Results', 'Threshold match pairs',                    CAST(MS.Threshold_Pairs     AS NVARCHAR(100)) FROM MatchedStats MS UNION ALL
SELECT '4. Matching Results', 'Intra-flagged transactions (Intra_Flag=1)',CAST(B.Flagged_Count AS NVARCHAR(50)) + ' (' + CAST(CAST(ROUND(100.0 * B.Flagged_Count / NULLIF(B.Total_Txns, 0), 2) AS DECIMAL(10,2)) AS NVARCHAR(20)) + '%)' FROM BaseAgg B UNION ALL
SELECT '4. Matching Results', 'Non-flagged transactions (Intra_Flag=0)',  CAST(B.Unflagged_Count AS NVARCHAR(50)) + ' (' + CAST(CAST(ROUND(100.0 * B.Unflagged_Count / NULLIF(B.Total_Txns, 0), 2) AS DECIMAL(10,2)) AS NVARCHAR(20)) + '%)' FROM BaseAgg B UNION ALL
-- 5. Run Configuration
SELECT '5. Run Configuration', 'Source table',                            @TableName UNION ALL
SELECT '5. Run Configuration', 'Day threshold used',                      CAST(@DayThreshold    AS NVARCHAR(100)) UNION ALL
SELECT '5. Run Configuration', 'Amount threshold used',                   CAST(@AmountThreshold AS NVARCHAR(100));

    -- 4B. Open a high-performance cursor over UNMATCHED debits only
    DECLARE debit_cursor CURSOR LOCAL FAST_FORWARD FOR
    SELECT Transaction_Row_ID, Customer_Number, Account_Number, Amount, Txn_Date
    FROM #Debits
    WHERE Matched_Group_ID IS NULL;

        -- 4D. Find the absolute closest available credit
        SELECT TOP (1) @FoundCreditRowID = Transaction_Row_ID
        FROM #Credits
        WHERE Matched_Group_ID IS NULL -- Ensures credit is not already taken
          AND Customer_Number = @CurrentCust
          AND Account_Number <> @CurrentAcc
          -- Preserving your original logic: Credit must happen on or after Debit date
          AND Txn_Date >= @CurrentDate 
          AND Txn_Date <= DATEADD(DAY, @DayThreshold, @CurrentDate)
          AND ABS(Amount - @CurrentAmt) <= @AmountThreshold
        ORDER BY 
          ABS(DATEDIFF(DAY, Txn_Date, @CurrentDate)) ASC, -- Prioritize closest date
          ABS(Amount - @CurrentAmt) ASC;