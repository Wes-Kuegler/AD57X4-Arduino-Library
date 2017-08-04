# -*- coding: utf-8 -*-
"""
Created on Mon Jul 3 08:58:10 2017

@author: Wesley Kuegler, NIFS Summer 2017 Intern at Nasa's Langley Research Center
This file contains functions for plotting output data from the MEDLI2 SSE Sensor Simulator in various ways. To 
run the scripts on other datasets, simply change the "inFile" and "outFile" paths near the bottom of this file. 

==============================================IN THIS VERSION, V17.8.1=============================================
*renamed charts on Max Variance sheets inside expectedVsVariance() to more accurately reflect what they are
    -also changed those charts from connected scatter graphs to regular scatter graphs

=============================================OLDER VERSION INFORMATION=============================================
======================================================V17.7.31=====================================================
*changed largeErrorHandler to simply return none if the test value is larger than 10^18
*bug fix: expectedVsError() now .applies largeErrorHandler() to the error values instead of the expected values. 
    The massive error issue in the graphs is now fixed

======================================================V17.7.27=====================================================
*added another dataframe to expectedVsError() so the temperatures data can be further manipulated without
    altering dataframe dfT

======================================================V17.7.24=====================================================
*fixed a series naming error in time vs measured
*switched plot types to scatter for all plots, some of which have markers (set to circles)
*moved x-axis labels to the bottom of all graphs
####ISSUES####
    *in line 335, still trying to decide how to select the range of y-values that should be visible on the error
    graphs

======================================================V17.7.21=====================================================
*changed the out-of-bounds value from -274 to "None" because Excel will ignore "None" completely
*fixed series naming in expectedVsMeasured()
*changed the style of chart for all functions from "line" to "scatter", with the subtype "straight_with_markers."
    markers will need to be changed to a standard shape, like circle. Right now it's a little crowded and confusing
    
======================================================V17.7.19=====================================================
*created function zeroDivHandler() for removing inf and -inf values resulting from zero division
*added zero division handling to expectedVsError(V) with zeroDivHandler()
    STILL NEED TO ADD TO OTHERS
    
======================================================V17.7.18=====================================================
*removed expectedVsExpected()
*removed "tref" argument from typeK.inverse call in toTemp()
*in expectedVsDiff(), the difference is no longer the absolute difference. The difference is now expected voltages
    minus measured voltages
*fixed series naming - "TC010" is now "TC10"
*expectedVsMeasured() now sorts the dataframe by InputRow instead of TC01's expected values
*timeVsMeasured() no longer sorts by time, as the dataframe is already in chronological order
*expectedVsDiff() renamed to expectedVsVariance()
*expectedVsVariance() now sorts the dataframe by InputRow instead of TC01's expected values
*added a try...except loop for the excel file saving at the end of the code. This is so there is an opportunity to
    close an open excel file and allow the code to overwrite it, rather than having the program crash and need to
    be run again
    
======================================================V17.7.17=====================================================
*Added try...except functionality to trimTime() so that it won't crash if the times is already formatted correctly
*Code from expectedVsMeasured() moved to timeVsMeasured(), and completed. The measured values are now graphed 
    against time and broken up by their input row in order to see the ability of the channels to hold their values
    over time
    
======================================================V17.7.13=====================================================
*Added the function trimTime(), which pulls only the hour and minute from the time columns in a dataframe.
    trimTime() is applied to the primary dataframe in main
*Added meaningful series names for each chart 
*Began adding code in expectedVsMeasured() to separate the data by set point, and plot the new sets individually
    *NOT YET WORKING

======================================================V17.7.12=====================================================
*Added the function trimTime(), which pulls only the hour and minute from the time columns in a dataframe.
    trimTime() is applied to the primary dataframe in main
*Added meaningful series names for each chart 
*Began adding code in expectedVsMeasured() to separate the data by set point, and plot the new sets individually
    *NOT YET WORKING
    
======================================================V17.7.11=====================================================
*Dataframe constructor errors resolved in expectedVsDiff()
*Added code in expectedVsDiff() to find and store the voltage and temperature maximum variances. These values are
    stored in sheets following the voltage and temperature difference sheets
*Changed rowCount from hard-coded to being read from the dataframe length
*toTemp() now returns -274 (just below absolute zero) if the voltage is <-7.45. This is because the typeK
    thermocouples do not operate below -7.45 volts, but we may simulate lower voltages for testing. Therefore we
    don't need the values to be converted to temps, but we do need to use SOME placeholder that makes it obvious
    that the original voltage was out of bounds. -274 is clearly an invalid temperature.
*Added chart titles
*Added charts for the max variance sheets

======================================================V17.7.10=====================================================
*Consolidated all outputs into one .xlsx file. Each function stores its graphs on new sheets
*Added a runAll() function to run all of the analysis functions
*Moved dataframe and other initialization calls out of the functions and into the main. Added a "df" dataframe
    parameter to each analysis function
*Consolidated expectedVsTempDiff() and expectedVsVoltDiff() into expectedVsDiff()
*Renamed "df" to "dfV" and added "dfT" - now there is a dataframe each for voltages and temperatures
*Each analysis function now performs the same analysis twice: once for the temperatures, and once for the voltages.
    The graphs are placed on different sheets, named with (T) or (V) to indicate temperatures or voltages
##IN PROGRESS##
    *in expectedVsDiff(), code (not working yet) has been added to make a new dataframe for the min/max variance
        values. Currently the constructor is being called incorrectly
        
======================================================V17.7.5======================================================
*The expected voltages ("TC01ExpectedVolt, TC02ExpectedVolt, ...") can now be plotted against themselves
    using the expectedVsExpected() function
*The expected voltages ("TC01ExpectedVolt, TC02ExpectedVolt, ...") can now be plotted against the measured
    voltages ("TC01MeasuredVolt, TC02MeasuredVolt, ...) using the expectedVsMeasured() function
*The expected temperatures ("TC01ExpectedVolt, TC02ExpectedVolt, ...") can now be plotted against the absolute
    values of the differences between the measured and expected temperatures using the expectedVsDiff() function
*The expected temperatures can be plotted over time using the timeVsExpected()  function
*The Measured temperatures can now be plotted over time using the tmieVsMeasured() function
*The expected values can be plotted against the %error of the measured values
*The expected voltages can be plotted against the absolute values of the differences between the measured and 
    expected voltages

"""

#imports
import pandas as pd
import numpy as np
from thermocouples_reference import thermocouples

#converts a voltage to a temperature using thermocouples_reference
def toTemp(voltage):
    try:
        return typeK.inverse_CmV(voltage * 100)
    except:
        return None    #return a value that Excel will ignore
        
#trims the time values of a dataframe to just hours, minutes, seconds. takes a string
def trimTime(s):
    try:
        x = s.split(" ")
        return x[1].split('.')[0]
    
    except:
        pass
        
#converts a value of "inf" or "-inf" (from a division by zero) to "None"
def zeroDivHandler(value):
    if value == np.inf or value == -np.inf:
        return None
    else:
        return value
    
#delete error values greater than 10^18 or less than -10^18
def largeErrorHandler(error):
    if (error > 1000000000000000000 or error < -1000000000000000000):
        return None
    else:
        return error

#plots the expected voltages against the measured voltages
def expectedVsMeasured(dfV, dfT, inFile, seriesCount, rowCount):         
    dfV = dfV.sort_values(["InputRow"])       #sort the rows by the expected voltages
        
    sheet = "expectedVsMeasured(V)"
    dfV.to_excel(writer, sheet_name = sheet)
    worksheet = writer.sheets[sheet]    #make the worksheet
    chart = workbook.add_chart({'type': 'scatter', 'subtype': 'straight'})    #create a chart object
    
    for x in range (seriesCount):    #configure series
        chart.add_series({
                    'categories': [sheet, 1, 6 + (4 * x), rowCount, 6 + (4 * x)],      #expected voltages
                    'values': [sheet, 1, 5 + (4 * x), rowCount, 5 + (4 * x)],          #measured voltages
                    'name': ("TC0" + str(x+1)) if x < 9 else ("TC" + str(x+1)),        #series name
                    })
        
    chart.set_x_axis({'name': 'Set Voltage', 'label_position': 'low'})  
    chart.set_y_axis({'name': 'Measured Voltage'})          #configure chart y axis
    chart.set_title({"name" : "Expected Voltages vs Measured Voltages"})
    worksheet.insert_chart('D20', chart)    #insert the chart into the worksheet    
    
    #repeat for the temperatures dataframe      
    dfT = dfT.sort_values(["InputRow"])       #sort the rows by the input row
        
    sheet = "expectedVsMeasured(T)"
    dfT.to_excel(writer, sheet_name = sheet)
    worksheetT = writer.sheets[sheet]    #make the worksheet
    chartT = workbook.add_chart({'type': 'scatter', 'subtype': 'straight'})     #create a chart object
    
    for x in range (seriesCount):    #configure series
        chartT.add_series({
                    'categories': [sheet, 1, 6 + (4 * x), rowCount, 6 + (4 * x)],      #expected voltages
                    'values': [sheet, 1, 5 + (4 * x), rowCount, 5 + (4 * x)],          #measured voltages
                    'name': ("TC0" + str(x+1)) if x < 9 else ("TC" + str(x+1)),        #series name
                    })
        
    chartT.set_x_axis({'name': 'Set Voltage', 'label_position': 'low'})       
    chartT.set_y_axis({'name': 'Measured Temperature'})       #configure chart y axis
    chartT.set_title({"name" : "Expected Temperatures vs Measured Temperatures"})
    worksheetT.insert_chart('D20', chartT)    #insert the chart into the worksheet           
    
#plots the time against the expected values
def timeVsExpected(dfV, dfT, inFile, seriesCount, rowCount):      
    sheet = "timeVsExpected(V)"
    dfV.to_excel(writer, sheet_name = sheet)
    worksheet = writer.sheets[sheet]    #make the worksheet
    chart = workbook.add_chart({'type': 'scatter', 'subtype': 'straight'})    #create a chart object
            
    for x in range (seriesCount):    #configure series
        chart.add_series({
                    'categories': [sheet, 1, 8 + (4 * x), rowCount, 8 + (4 * x)],  #time
                    'values': [sheet, 1, 6 + (4 * x), rowCount, 6 + (4 * x)],      #expected voltages
                    'name': ("TC0" + str(x+1)) if x < 9 else ("TC" + str(x+1))    #series name
                    })
    
    chart.set_x_axis({'name': 'Time', 'label_position': 'low'})              #configure chart x axis
    chart.set_y_axis({'name': 'Set Voltage'})       #configure chart y axis
    chart.set_title({"name" : "Time vs Expected Voltages"})
    
    worksheet.insert_chart('D20', chart)    #insert the chart into the worksheet
    
    #repeat for the temperatures dataframe
    sheet = "timeVsExpected(T)"
    dfT.to_excel(writer, sheet_name = sheet)
    worksheetT = writer.sheets[sheet]    #make the worksheet
    chartT = workbook.add_chart({'type': 'scatter', 'subtype': 'straight'})    #create a chart object
            
    for x in range (seriesCount):    #configure series
        chartT.add_series({
                    'categories': [sheet, 1, 8 + (4 * x), rowCount, 8 + (4 * x)],  #time
                    'values': [sheet, 1, 6 + (4 * x), rowCount, 6 + (4 * x)],      #expected voltages
                    'name': ("TC0" + str(x+1)) if x < 9 else ("TC" + str(x+1))    #series name
                    })
    
    chartT.set_x_axis({'name': 'Time', 'label_position': 'low'})              #configure chart x axis
    chartT.set_y_axis({'name': 'Set Temperature'})       #configure chart y axis
    chartT.set_title({"name" : "Time vs Expected Temperatures"})

    worksheetT.insert_chart('D20', chartT)    #insert the chart into the worksheet
       
#plots the time vs the measured values
def timeVsMeasured(dfV, dfT, inFile, seriesCount, rowCount):            
    sheet = "timeVsMeasured(V)"
    dfV.to_excel(writer, sheet_name = sheet)    
    worksheet = writer.sheets[sheet]    #make the worksheet    
    chart = workbook.add_chart({'type': 'scatter', 'subtype': 'straight'})    #create a chart object
            
    for x in range (seriesCount):    #configure series
        chart.add_series({
                    'categories': [sheet, 1, 8 + (4 * x), rowCount, 8 + (4 * x)],  #time
                    'values': [sheet, 1, 5 + (4 * x), rowCount, 5 + (4 * x)],      #measured voltages
                    'name': ("TC0" + str(x+1)) if x < 9 else ("TC" + str(x+1))    #series name
                    })
        
    chart.set_x_axis({'name': 'Time', 'label_position': 'low'})              #configure chart x axis
    chart.set_y_axis({'name': 'Measured Voltage'})       #configure chart y axis
    chart.set_title({"name" : "Time vs Measured Voltages"})
    
    worksheet.insert_chart('D20', chart)    #insert the chart into the worksheet
 
    #repeat for the temperatures dataframe
    sheet = "timeVsMeasured(T)"
    dfT.to_excel(writer, sheet_name = sheet)    
    worksheetT = writer.sheets[sheet]    #make the worksheet    
    chartT = workbook.add_chart({'type': 'scatter', 'subtype': 'straight'})    #create a chart object
            
    for x in range (seriesCount):    #configure series
        chartT.add_series({
                    'categories': [sheet, 1, 8 + (4 * x), rowCount, 8 + (4 * x)],  #time
                    'values': [sheet, 1, 5 + (4 * x), rowCount, 5 + (4 * x)],      #measured voltages
                    'name': ("TC0" + str(x+1)) if x < 9 else ("TC" + str(x+1))    #series name
                    })
        
    chartT.set_x_axis({'name': 'Time', 'label_position': 'low'})              #configure chart x axis
    chartT.set_y_axis({'name': 'Measured Temperature'})       #configure chart y axis
    
    chartT.set_title({"name" : "Time vs Measured Temperatures"})
    
    worksheetT.insert_chart('D20', chartT)    #insert the chart into the worksheet
    
    #now break up the voltage data based on the set point. Plot the new set point fragments

    partitions = [0]     #array of indices at which there is a change in the "input row" column
    for x in range(rowCount):       #loop through the rows to partition them by input row
        if x == rowCount-1 or dfV.iloc[x, 0] != dfV.iloc[x+1, 0]:  #if the next input row value is different
            partitions.append(x+1)      #mark this index as the end of a partition, +1 because the series indexing is off by 1
                
    sheet = "timeVsMeasured - Ind.(V)"
    dfV.to_excel(writer, sheet_name = sheet)
    IVworksheet = writer.sheets[sheet]    #make the worksheet
        
    for x in range(1, len(partitions)):
        IVchart = workbook.add_chart({'type': 'scatter', 'subtype': 'straight'})    #create a chart object for the individual set voltages
        
        for y in range (seriesCount):    #configure series
            IVchart.add_series({
                        'categories': [sheet, partitions[x-1]+1, 8 + (4 * y), partitions[x], 8 + (4 * y)],      #expected voltages
                        'values': [sheet, partitions[x-1]+1, 5 + (4 * y), partitions[x], 5 + (4 * y)],          #measured voltages
                        'name': ("TC0" + str(y+1)) if y < 9 else ("TC" + str(y+1)),        #series name
                        'marker': {'type': 'circle'}
                    })
        
        IVchart.set_x_axis({'name': 'Time', 'label_position': 'low'})       #configure chart x axis
        IVchart.set_y_axis({'name': 'Measured Voltage'})              #configure chart y axis
        IVchart.set_title({'name': 'Input Row ' + str(x)})    #configure chart title
        IVworksheet.insert_chart('D' + str(x * 15), IVchart)    #insert the chart into the worksheet
        
    #now break up the temperature data based on the set point. Plot the new set point fragments

    partitions = [0]     #array of indices at which there is a change in the "input row" column
    for x in range(rowCount):       #loop through the rows to partition them by input row
        if x == rowCount-1 or dfT.iloc[x, 0] != dfT.iloc[x+1, 0]:  #if the next input row value is different
            partitions.append(x+1)      #mark this index as the end of a partition, +1 because the series indexing is off by 1
    
    sheet = "timeVsMeasured - Ind.(T)"
    dfT.to_excel(writer, sheet_name = sheet)
    ITworksheet = writer.sheets[sheet]    #make the worksheet
        
    for x in range(1, len(partitions)):
        ITchart = workbook.add_chart({'type': 'scatter', 'subtype': 'straight'})    #create a chart object for the individual set voltages
        
        for y in range (seriesCount):    #configure series
            ITchart.add_series({
                        'categories': [sheet, partitions[x-1]+1, 8 + (4 * y), partitions[x], 8 + (4 * y)],      #expected voltages
                        'values': [sheet, partitions[x-1]+1, 5 + (4 * y), partitions[x], 5 + (4 * y)],          #measured voltages
                        'name': ("TC0" + str(y+1)) if y < 9 else ("TC" + str(y+1)),        #series name
                        'marker': {'type': 'circle'}
                    })
        
        ITchart.set_x_axis({'name': 'Time', 'label_position': 'low'})       #configure chart x axis
        ITchart.set_y_axis({'name': 'Measured Temperature'})              #configure chart y axis
        ITchart.set_title({'name': 'Input Row ' + str(x)})    #configure chart title
        ITworksheet.insert_chart('D' + str(x * 15), ITchart)    #insert the chart into the worksheet
        
#plots the expected values vs the %error of the absolute value difference 
def expectedVsError(dfV, dfT, inFile, seriesCount, rowCount):    
    diff = [None] * seriesCount      #list for storing the difference columns 
    for x in range (0, seriesCount):
        diff[x] = dfV.iloc[:, 4 + (4 * x)]   #store measured voltage column copies in diff
        diff[x] = dfV.iloc[:, 5 + (4 * x)] - diff[x]    #take the differences and store them as dataframe columns in diff    
        diff[x] = diff[x] / dfV.iloc[:, 5 + (4 * x)]     #divide the difference by the theoretical value to find the %error
        diff[x] = diff[x].apply(zeroDivHandler)
        dfV["%error" + (str(x))] = pd.Series(diff[x])    #put the %error columns at the end of the excel file
    
    dfV = dfV.sort_values(["TC01ExpectedVolt"])       #sort the rows by the expected voltages
    
    sheet = "expectedVsError(V)"
    dfV.to_excel(writer, sheet_name = sheet)
    worksheet = writer.sheets[sheet]    #make the worksheet
    chart = workbook.add_chart({'type': 'scatter', 'subtype': 'straight'})    #create a chart object
            
    for x in range (seriesCount):    #configure series
        chart.add_series({
                    'categories': [sheet, 1, 6 + (4 * x), rowCount, 6 + (4 * x)],  #expected voltages
                    'values': [sheet, 1, dfV.columns.get_loc("%error0") + x + 1, rowCount, dfV.columns.get_loc("%error0") + x + 1],  #%error of measured voltages
                    'name': ("TC0" + str(x+1)) if x < 9 else ("TC" + str(x+1))    #series name
                    })
        
    chart.set_x_axis({'name': 'Set Voltage', 'label_position': 'low'})       #configure chart x axis
    chart.set_y_axis({'name': 'Percentage Error of Measured Voltages'})       #configure chart y axis 
    chart.set_title({"name" : "Expected Voltages vs Percent Error"})
    worksheet.insert_chart('D20', chart)    #insert the chart into the worksheet
    
    #repeat for the temperatures dataframe
    diffT = [None] * seriesCount      #list for storing the difference columns 
    
    dfT_corrected = dfT.copy()        #make a copy of the dataframe so we can manipulate it without changing dfT
          
    for x in range (0, seriesCount):
            diffT[x] = dfT_corrected.iloc[:, 4 + (4 * x)]   #store measured voltage column copies in diff
            diffT[x] = dfT_corrected.iloc[:, 5 + (4 * x)] - diffT[x]    #take the differences and store them as dataframe columns in diff              
            diffT[x] = diffT[x] / dfT_corrected.iloc[:, 5 + (4 * x)]     #divide the difference by the theoretical value to find the %error
            diffT[x] = diffT[x].apply(zeroDivHandler)
            dfT_corrected["%error" + (str(x))] = pd.Series(diffT[x])    #put the %error columns at the end of the excel file
            
    dfT_corrected = dfT_corrected.sort_values(["TC01ExpectedVolt"])         #sort the rows by the expected temperatures
    for x in range (seriesCount):                                           #remove very large values from the error columns
        dfT_corrected.iloc[:, dfT_corrected.columns.get_loc("%error0") + x] = dfT_corrected.iloc[:, dfT_corrected.columns.get_loc("%error0") + x].apply(largeErrorHandler)

    sheet = "expectedVsError(T)"
    dfT_corrected.to_excel(writer, sheet_name = sheet)
    worksheetT = writer.sheets[sheet]    #make the worksheet
    chartT = workbook.add_chart({'type': 'scatter', 'subtype': 'straight'})   #create a chart object
            
    for x in range (seriesCount):    #configure series
        chartT.add_series({
                    'categories': [sheet, 1, 6 + (4 * x), rowCount, 6 + (4 * x)],  #expected temperatures
                    'values': [sheet, 1, dfT_corrected.columns.get_loc("%error0") + x + 1, rowCount, dfT_corrected.columns.get_loc("%error0") + x + 1],  #%error of measured temperatures
                    'name': ("TC0" + str(x+1)) if x < 9 else ("TC" + str(x+1))    #series name
                    })
        
    chartT.set_x_axis({'name': 'Set Temperature', 'label_position': 'low'})       #configure chart x axis
    chartT.set_y_axis({'name': 'Percentage Error of Measured Temperatures'})       #configure chart y axis
    chartT.set_title({"name" : "Expected Temperatures vs Percent Error"})
    worksheetT.insert_chart('D20', chartT)    #insert the chart into the worksheet
        
#plots the expected values against the absolute difference between the expected and measured values
def expectedVsVariance(dfV, dfT, inFile, seriesCount, rowCount):
    diff = [None] * seriesCount      #list for storing the difference columns 
    for x in range (0, seriesCount):
        diff[x] = dfV.iloc[:, 4 + (4 * x)]   #store measured voltage column copies in diff
        diff[x] = dfV.iloc[:, 5 + (4 * x)] - diff[x]    #take the differences and store them as dataframe columns in diff
#        diff[x] = diff[x].apply(abs)                    #take the absolute value of the differences
        dfV["diff" + (str(x))] = pd.Series(diff[x])      #put the diff columns at the end of the excel file
            
    dfV = dfV.sort_values(["InputRow"])       #sort the rows by the expected voltages
    
    sheet = "expectedVsVariance(V)"
    dfV.to_excel(writer, sheet_name = sheet)
    worksheet = writer.sheets[sheet]    #make the worksheet
    chart = workbook.add_chart({'type': 'scatter', 'subtype': 'straight'})    #create a chart object
            
    for x in range (seriesCount):    #configure series
        chart.add_series({
                    'categories': [sheet, 1, 6 + (4 * x), rowCount, 6 + (4 * x)],  #expected voltages
                    'values': [sheet, 1, dfV.columns.get_loc("diff0") + x + 1, rowCount, dfV.columns.get_loc("diff0") + x + 1],      #measured minus expected voltages
                    'name': ("TC0" + str(x+1)) if x < 9 else ("TC" + str(x+1))    #series name
                    })
        
    chart.set_x_axis({'name': 'Set Voltage', 'label_position': 'low'})       #configure chart x axis
    chart.set_y_axis({'name': 'Diff Between Expected and Measured Voltages'})       #configure chart y axis
    chart.set_title({"name" : "Expected Voltages vs Variances"})
    
    worksheet.insert_chart('D20', chart)    #insert the chart into the worksheet
    
    #find the max voltage differences, write into another sheet    
    dfD = pd.DataFrame(columns = ["Set Point", "Max Variance"], index = [None] * seriesCount)
    sheet = "Max Variances(V)"
    
    for x in range (seriesCount):           #find the max variance for each series
        dfD.iloc[x, 0] = dfV.iloc[0, 5 + (x * 4)]        #assumes that each series has the same expected voltage for every row
        dfD.iloc[x, 1] = dfV.iloc[:, dfV.columns.get_loc("diff0") + x].max()
    
    dfD.to_excel(writer, sheet_name = sheet)
    worksheetDV = writer.sheets[sheet]    #make the worksheet
    chartDV = workbook.add_chart({'type': 'scatter'})    #create a chart object
    
    chartDV.add_series({
            'categories': [sheet, 1, 1, len(dfD), 1],  #expected voltages
            'values': [sheet, 1, 2, len(dfD), 2],      #max variances from expected values
            'name': "Voltage Variance"
            })
        
    chartDV.set_x_axis({'name': 'Set Voltage', 'label_position': 'low'})       #configure chart x axis
    chartDV.set_y_axis({'name': 'Diff Between Expected and Measured Voltages'})       #configure chart y axis
    chartDV.set_title({"name" : "Max Variances vs Set Voltage"})
   
    worksheetDV.insert_chart('D20', chartDV)    #insert the chart into the worksheet    
    
    #plot the expected values against the absolute difference for the temperatures dataframe
    diffT = [None] * seriesCount      #list for storing the difference columns 
    for x in range (0, seriesCount):
        diffT[x] = dfT.iloc[:, 4 + (4 * x)]   #store measured voltage column copies in diff
        diffT[x] = dfT.iloc[:, 5 + (4 * x)] - diffT[x]     #take the differences and store them as dataframe columns in diff
        dfT["diff" + (str(x))] = pd.Series(diffT[x])      #put the diff columns at the end of the sheet
            
    dfT = dfT.sort_values(["InputRow"])       #sort the rows by the expected voltages
    
    sheet = "expectedVsVariance(T)"
    dfT.to_excel(writer, sheet_name = sheet)
    worksheetT = writer.sheets[sheet]    #make the worksheet
    chartT = workbook.add_chart({'type': 'scatter', 'subtype': 'straight'})    #create a chart object
            
    for x in range (seriesCount):    #configure series
        chartT.add_series({
                    'categories': [sheet, 1, 6 + (4 * x), rowCount, 6 + (4 * x)],  #expected voltages
                    'values': [sheet, 1, dfT.columns.get_loc("diff0") + x + 1, rowCount, dfT.columns.get_loc("diff0") + x + 1],      #measured minus expected voltages
                    'name': ("TC0" + str(x+1)) if x < 9 else ("TC" + str(x+1))    #series name
                    })
        
    chartT.set_x_axis({'name': 'Set Temperature', 'label_position': 'low'})       #configure chart x axis
    chartT.set_y_axis({'name': 'Diff Between Expected and Measured Temperatures'})       #configure chart y axis
    chartT.set_title({"name" : "Expected Temperatures vs Variances"})
    worksheetT.insert_chart('D20', chartT)    #insert the chart into the worksheet
    
     #find the max temperature differences, write into another sheet        
    dfD = pd.DataFrame(columns = ["Set Point", "Max Variance"], index = [None] * seriesCount)
    sheet = "Max Variances(T)"
    
    for x in range (seriesCount):           #find the max variance for each series
        dfD.iloc[x, 0] = dfT.iloc[0, 5 + (x * 4)]        #assumes that each series has the same expected voltage for every row
        dfD.iloc[x, 1] = dfT.iloc[:, dfT.columns.get_loc("diff0") + x].max()
        
    dfD.to_excel(writer, sheet_name = sheet)
    worksheetDT = writer.sheets[sheet]    #make the worksheet
    chartDT = workbook.add_chart({'type': 'scatter'})    #create a chart object
    
    chartDT.add_series({
            'categories': [sheet, 1, 1, len(dfD), 1],  #expected voltages
            'values': [sheet, 1, 2, len(dfD), 2],      #max variances from expected values
            'name': "Temperature Variances"
            })
        
    chartDT.set_x_axis({'name': 'Set Temperature', 'label_position': 'low'})       #configure chart x axis
    chartDT.set_y_axis({'name': 'Diff Between Expected and Measured Temperatures'})       #configure chart y axis
    chartDT.set_title({"name" : "Max Variances vs Set Temperature"})
   
    worksheetDT.insert_chart('D20', chartDT)    #insert the chart into the worksheet    
    
   

#run all of the data analysis functions    
def runAll(dfV, dfT, inFile, seriesCount, rowCount, toTemp):
    expectedVsMeasured(dfV, dfT, inFile, seriesCount, rowCount)
    timeVsExpected(dfV, dfT, inFile, seriesCount, rowCount)
    timeVsMeasured(dfV, dfT, inFile, seriesCount, rowCount)
    expectedVsError(dfV, dfT, inFile, seriesCount, rowCount)
    expectedVsVariance(dfV, dfT, inFile, seriesCount, rowCount)
    
typeK = thermocouples['K']                  #set up thermocouples typeK conversion

pd.set_option('display.mpl_style', 'default')       #Make the graphs a bit prettier

seriesCount = 29      #the number of channels in the input file to make into chart series
inFile = 'Mike Plots/Output_Final_csv_2017-08-01_14h57m36s_(InputCSV_LinearStepTopHalfRange_0P001_Step)_2.csv'       #define the file name for the input file
outFile = 'Mike Plots/Output_Final_csv_2017-08-01_14h57m36s_(InputCSV_LinearStepTopHalfRange_0P001_Step)_2.xlsx'     #file name for processed output
    
dfV = pd.read_csv(inFile)      #import the output data as a new data frame. This one is for voltages 
rowCount = len(dfV)            #the number of rows in the input file
writer = pd.ExcelWriter(outFile, engine = 'xlsxwriter')    #initialize the writer
workbook = writer.book      #make the workbook

for x in range (seriesCount):   #trim the time values
    dfV.iloc[:, 7 + (4 * x)] = dfV.iloc[:, 7 + (4 * x)].apply(trimTime)

dfT = dfV.copy()       #make a copy of the dataframe. This one is for temperatures
for x in range (seriesCount):     #convert all expected voltage values to temperatures
    dfT.iloc[:, 5 + (4 * x)] = dfT.iloc[:, 5 + (4 * x)].apply(toTemp)
    
for x in range (seriesCount):     #convert all measured voltage values to temperatures
    dfT.iloc[:, 4 + (4 * x)] = dfT.iloc[:, 4 + (4 * x)].apply(toTemp)
    

runAll(dfV, dfT, inFile, seriesCount, rowCount, False)

writer.save()   #close the Pandas Excel writer, output the Excel file