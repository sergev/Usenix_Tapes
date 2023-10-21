# 
# This is a 1040 template (1986 Federal taxes) for the SC spreadsheet
# posted recently.  It is quite basic: has the 1040, schedule A,
# and schedule W.  No tax tables or any cleverness, but it may
# be a starting point for others.
# 
# I used this for my taxes, but there are no guarantees of
# correctness - use at your own risk, etc..  Cut at the line
# below and feed this to the sc spreadsheet program.
# 
# Steve Spearman  ihnp4!ihlpf!spear
# 
# --------cut here----------
# This data file was generated by the Spreadsheet Calculator.
# You almost certainly shouldn't edit it.

format A 30 2
format B 30 2
format C 30 2
format D 30 2
format E 30 2
format G 30 2
format H 30 2
leftstring A0 = " 1040 TAX FORM SHEET"
leftstring B0 = " 1040 TAX FORM SHEET"
leftstring C0 = " 1040 TAX FORM SHEET"
leftstring D0 = " 1040 TAX FORM SHEET"
leftstring E0 = "SCHEDULE W - MARRIED COUPLE"
leftstring G0 = " SCHEDULE A- ITEMIZED"
leftstring H0 = "DEDUCTIONS"
leftstring B1 = " 6f. Total Exemptions"
let B1 = 0
rightstring E1 = "You"
rightstring F1 = "Spouse"
leftstring B2 = " 7. Wages, etc."
let B2 = 0
leftstring D2 = " 33. Gross (L.32)"
let D2 = B29
leftstring E2 = " 1. Wages, tips, etc."
let E2 = 0
let F2 = 0
leftstring G2 = " 1. Medicines"
let G2 = 0
leftstring B3 = " 8. Interest Income"
let B3 = 0
leftstring D3 = " 34a. Sch.A L.26"
let D3 = H32
leftstring E3 = " 2. Self. Empl., etc."
let E3 = 0
let F3 = 0
leftstring G3 = " 2a. Medical dental"
let G3 = 0
leftstring A4 = "9a. Dividends"
let A4 = 0
leftstring C4 = " 34b. Cash Contrib."
let C4 = 0
leftstring E4 = " 3. Total Earned"
let E4 = E2+E3
let F4 = F2+F3
leftstring G4 = " 2b. transport/lodge"
let G4 = 0
leftstring A5 = "9b. Exclusion"
let A5 = 0
leftstring B5 = " 9c. Dividend 9a-9b"
let B5 = A4-A5
leftstring C5 = " 34c. Noncash "
let C5 = 0
leftstring G5 = " 2c. other"
let G5 = 0
leftstring B6 = " 10. Taxable refunds"
let B6 = 0
leftstring C6 = " 34d. 34b + 34c"
let C6 = C4+C5
leftstring D6 = " 34e. 34d / 2"
let D6 = C6/2
leftstring E6 = " 4. Adjustments"
let E6 = 0
let F6 = 0
leftstring G6 = " 3. Total"
let G6 = @sum(G2:G5)
leftstring B7 = " 11. Alimony "
let B7 = 0
leftstring D7 = " 35. 33-(34a or 34e)"
let D7 = D2-D3-D6
leftstring E7 = " 5. Qualified income"
let E7 = E4-E6
let F7 = F4-F6
leftstring G7 = " 4. F.1040 L.33 * .5"
let G7 = D2*0.05
leftstring B8 = " 12. Business income"
let B8 = 0
leftstring D8 = " 36. Exemptions*1080"
let D8 = B1*1080
leftstring H8 = " 5. L3 - L4 or 0"
let H8 = G6-G7>0?G6-G7:0
leftstring B9 = " 13. Capital gain"
let B9 = 0
leftstring D9 = " 37. Taxable Income"
let D9 = D7-D8
leftstring E9 = " 6. Smaller of 5"
let E9 = E7<F7?E7:F7
leftstring G9 = " 6. State/local tax"
leftstring B10 = " 14. 40% cap. gain"
let B10 = 0
leftstring D10 = " 38. ENTER TAX"
leftstring E10 = " 7. Percentage used"
let E10 = 0.1
leftstring G10 = " 7. Real estate tax"
leftstring B11 = " 15. Other gains"
let B11 = 0
leftstring D11 = " 39. Addit. Taxes"
let D11 = 0
leftstring E11 = " 8. Deduction"
let E11 = E9*E10
leftstring G11 = " 8a. Sales tax"
leftstring B12 = " 16. Taxable pensions"
let B12 = 0
leftstring D12 = " 40. Total Taxes"
let D12 = D10+D11
leftstring G12 = " 8b. Motor vehicle"
let G12 = 0
leftstring A13 = "17a Other pensions"
let A13 = 0
leftstring B13 = " 17b. Taxable amount"
let B13 = 0
leftstring G13 = " 9. Other taxes"
let G13 = 0
leftstring B14 = " 18. Rents, etc."
let B14 = 0
leftstring C14 = " 41. Child care"
let C14 = 0
leftstring H14 = " 10. Total taxes"
let H14 = @sum(G9:G13)
leftstring B15 = " 19. Farm income"
let B15 = 0
leftstring C15 = " 42. Elderly credit"
let C15 = 0
leftstring G15 = " 11a. Mortgage Int."
let G15 = 0
leftstring A16 = "20a Unemployment"
let A16 = 0
leftstring B16 = " 20b. Taxable amount"
let B16 = 0
leftstring G16 = " 11b. Indiv. mortgage"
let G16 = 0
leftstring A17 = "21a. Social Sec."
let A17 = 0
leftstring B17 = " 21b. Taxable amount"
let B17 = 0
leftstring C17 = " 43. Political"
let C17 = 0
leftstring D17 = " 44. Total personal"
let D17 = @sum(C14:C17)
leftstring G17 = " 12. Credit card"
let G17 = 0
leftstring B18 = " 22. Other Income"
let B18 = 0
leftstring D18 = " 45. L.40 - L44"
let D18 = D12-D17>0?D12-D17:0
leftstring G18 = " 13. Other interest"
let G18 = 0
leftstring B19 = " 23. Total Income"
let B19 = @sum(B2:B18)
leftstring C19 = " 46. Foreign tax"
let C19 = 0
leftstring H19 = " 14. Total interest"
let H19 = @sum(G15:G18)
leftstring C20 = " 47. Business credit"
let C20 = 0
leftstring D20 = " 48. L.46 + L.47"
let D20 = C19+C20
leftstring G20 = " 15a Cash"
let G20 = 0
leftstring A21 = " 24. Moving Expense"
let A21 = 0
leftstring D21 = " 49. L.45 - L.48"
let D21 = D18-D20>0?D18-D20:0
leftstring G21 = " 15b. Cash > 3000"
let G21 = 0
leftstring A22 = " 25. Business Expense"
let A22 = 0
leftstring D22 = " 50. Self-empl. tax"
let D22 = 0
leftstring G22 = " 16. Non-cash"
let G22 = 0
leftstring A23 = " 26. IRA deduction"
let A23 = 0
leftstring D23 = " 51. Minimum tax"
let D23 = 0
leftstring G23 = " 17. Carryover"
let G23 = 0
leftstring A24 = " 27. Keogh deduction"
let A24 = 0
leftstring D24 = " 52. Invest. tax"
let D24 = 0
leftstring H24 = " 18. Total Contrib."
let H24 = @sum(G20:G23)
leftstring A25 = " 28. Savings penalty"
let A25 = 0
leftstring D25 = " 53. SS on tips"
let D25 = 0
leftstring H25 = " 19. Theft loss"
let H25 = 0
leftstring A26 = " 29. Alimony"
let A26 = 0
leftstring D26 = " 54. IRA tax"
let D26 = 0
leftstring G26 = " 20. Profess. Dues"
let G26 = 0
leftstring A27 = " 30. Married (Sch.W)"
let A27 = E11
leftstring D27 = " 55. Total Tax"
let D27 = @sum(D21:D26)
leftstring G27 = " 21. Tax prep. fee"
let G27 = 0
leftstring B28 = " 31. Total Adj."
let B28 = @sum(A21:A27)
leftstring C28 = " 56. Tax withheld"
let C28 = 0
leftstring G28 = " 22. Other"
let G28 = 0
leftstring B29 = " 32. Adjusted Gross"
let B29 = B19-B28
leftstring C29 = " 57. Est. tax"
let C29 = 0
leftstring H29 = " 23. Total misc."
let H29 = @sum(G26:G28)
leftstring C30 = " 58. Earned income"
let C30 = 0
leftstring H30 = " 24. Add above"
let H30 = H14+H19+H24+H25+H29
leftstring C31 = " 59. Form 4868"
let C31 = 0
leftstring H31 = " 25. zero bracket"
let H31 = 0
leftstring C32 = " 60. Excess SS"
let C32 = 0
leftstring H32 = " 26. L.24 - L.25"
let H32 = H30-H31
leftstring C33 = " 61. Fuel tax credit"
let C33 = 0
leftstring C34 = " 62. Reg. Invest. Co."
let C34 = 0
leftstring D34 = " 63. Total Payments"
let D34 = @sum(C28:C34)
leftstring D35 = " 64. Amount overpaid"
let D35 = D34>D27?D34-D27:0
leftstring D36 = " 65. Amount of Refund"
let D36 = D35-D37
leftstring D37 = " 66. apply est. tax"
let D37 = 0
leftstring D38 = " 67. Amount you owe"
let D38 = D27>D34?D27-D34:0


