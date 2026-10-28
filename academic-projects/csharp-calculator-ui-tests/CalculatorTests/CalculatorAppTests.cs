// *********************************************************
//
// Description: Appium UI tests for the Windows Calculator app.
// Author: Moshe Yifrah
//
// *********************************************************

using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using OpenQA.Selenium;
using OpenQA.Selenium.Appium;
using OpenQA.Selenium.Appium.Windows;
using Xunit;

namespace CalculatorTests
{
    public class CalculatorAppTests : IDisposable
    {
        private const string AppiumServerUrl = "http://127.0.0.1:4723";
        private const string CalculatorAppId = "Microsoft.WindowsCalculator_8wekyb3d8bbwe!App";
        private WindowsDriver<WindowsElement> driver;

        public CalculatorAppTests()
        {
            var appCapabilities = new AppiumOptions();
            appCapabilities.AddAdditionalCapability("app", CalculatorAppId);
            appCapabilities.AddAdditionalCapability("platformName", "Windows");
            appCapabilities.AddAdditionalCapability("deviceName", "WindowsPC");

            driver = new WindowsDriver<WindowsElement>(new Uri(AppiumServerUrl), appCapabilities);
        }

        public void Dispose()
        {
            driver?.Quit();
        }

        [Fact]
        public void TC1_ValidateCalculationResult()
        {
            // Step 1: Open Calculator (already done in setup)

            // Step 2: Perform calculation 4 + 15
            driver.FindElementByName("Four").Click();
            driver.FindElementByName("Plus").Click();
            driver.FindElementByName("One").Click();
            driver.FindElementByName("Five").Click();
            driver.FindElementByName("Equals").Click();

            // Step 3: Validate result
            var result = driver.FindElementByAccessibilityId("CalculatorResults").Text;
            Assert.Contains("19", result);
        }

        [Fact]
        public void TC2_ValidateCalculationHistory()
        {
            // Step 1: Open Calculator (already done in setup)

            // Step 2: Perform any calculation, e.g., 7 * 6
            driver.FindElementByName("Seven").Click();
            driver.FindElementByName("Multiply by").Click();
            driver.FindElementByName("Six").Click();
            driver.FindElementByName("Equals").Click();

            // Step 3: Open History tab
            driver.FindElementByName("Open Navigation").Click();
            driver.FindElementByName("History").Click();

            // Step 4: Validate the calculation is in history
            var historyItems = driver.FindElementsByClassName("ListViewItem");
            Assert.Contains(historyItems, item => item.Text.Contains("7 × 6 = 42"));
        }
    }
}

