# *********************************************************
#
# Description: A responsive web application designed to
# provide accurate and comprehensive weather forecasts.
#
# Authors: Itay Hamri, Romy Somkin, Shai Shaked, Moshe Yifrah
#
# *********************************************************

# --- Modules ---

"""
Modules overview:
- os: Allows interaction with the operating system (used to fetch env variables).
- sys: Provides access to system-specific parameters and functions (used for safe exits).
- dotenv: Loads environment variables from a .env file into the environment.
- flask: The web framework used to handle routing and render HTML templates.
- datetime, timezone: Used to handle, format, and compare dates and times (specifically for UTC comparisons).
- requests: Used to send HTTP requests to the external weather API.
- googlemaps: Client library for the Google Maps API (used for geocoding user input).
"""

import os
import sys
from dotenv import load_dotenv
from flask import Flask, render_template, request
from datetime import datetime, timezone
import requests
import googlemaps

# --- Globals ---

# Create a file named .env in the project root directory
# and insert exactly the following:
# GOOGLE_API_KEY=YOUR_API_KEY

load_dotenv()
API_KEY = os.environ.get("GOOGLE_API_KEY", "")

if not API_KEY:
    sys.exit(1)

google_maps = googlemaps.Client(key=API_KEY)
app = Flask(__name__)

# --- Implementation functions ---

@app.route('/', methods=['GET', 'POST'])
def home():
    """Handles routing for the home page and processes weather search form submissions."""
    user_location = None
    relevant_weather = None

    if request.method == 'POST':
        user_location = request.form.get("location") or None

        weather_result = get_weather_data(user_location)

        if weather_result:
            relevant_weather = get_relevant_data(user_location, weather_result)

    return render_template("index.html", location=user_location, weather=relevant_weather)


def get_weather_data(location):
    """Fetches the official location name via Google Maps and retrieves weather data."""

    geo_data = google_maps.geocode(location, language='en') # type: ignore

    if not geo_data:
        print("Location not found!")
        return None

    location_name = geo_data[0]['formatted_address']
    lat = geo_data[0]['geometry']['location']['lat']
    lon = geo_data[0]['geometry']['location']['lng']
    weather_url = f"https://weather.googleapis.com/v1/forecast/days:lookup?key={API_KEY}&location.latitude={lat}&location.longitude={lon}"

    try:
        weather_response = requests.get(weather_url, timeout=5)

        if weather_response.status_code == 200:
            weather_data = weather_response.json()
            weather_data['official_name'] = location_name
            return weather_data

    except requests.exceptions.RequestException as e:
        print(f"Network error: {e}")
        return None

    return None


def get_relevant_data(location, weather_data: dict | None):
    """Parses raw API data to extract current weather, day/night status, and a 5-day forecast."""

    if not weather_data:
        return None

    daily_forecast = weather_data.get("forecastDays", [])
    if not daily_forecast:
        return None

    today_data = daily_forecast[0]
    today_max = round(today_data.get("maxTemperature", {}).get("degrees", 0))
    today_min = round(today_data.get("minTemperature", {}).get("degrees", 0))
    today_humidity = today_data.get("daytimeForecast", {}).get("relativeHumidity")

    sun_events = today_data.get("sunEvents", {})
    sunrise = sun_events.get("sunriseTime")
    sunset = sun_events.get("sunsetTime")

    now_is_day = is_daytime(sunrise, sunset)

    if now_is_day:
        temp_now = today_max
    else:
        temp_now = today_min

    forecast_list = []

    for day_data in daily_forecast[:5]:
        date_dict = day_data.get("displayDate", {})
        year, month, day = date_dict.get("year"), date_dict.get("month"), date_dict.get("day")

        formatted_date, day_of_week = "N/A", "N/A"

        if year and month and day:
            formatted_date = f"{day:02d}/{month:02d}/{year}"
            dt = datetime(year, month, day)
            day_of_week = dt.strftime("%A")

        high = round(day_data.get("maxTemperature", {}).get("degrees", 0))
        low = round(day_data.get("minTemperature", {}).get("degrees", 0))

        humidity = day_data.get("daytimeForecast", {}).get("relativeHumidity")

        forecast_list.append({
            "date": formatted_date,
            "day": day_of_week,
            "humidity": humidity,
            "high_temp": high,
            "low_temp": low,
        })

    location_name = weather_data.get('official_name', location)

    result = {
        "city": location_name,
        "is_day_now": now_is_day,
        "humidity_now": today_humidity,
        "temp_now": temp_now,
        "forecast": forecast_list,
    }

    print("Result:", result)
    return result


def is_daytime(sunrise_str, sunset_str):
    """Determines if it is currently daytime at the requested location by comparing UTC times."""

    if not sunrise_str or not sunset_str:
        current_hour = datetime.now().hour
        return 6 <= current_hour < 18

    try:
        clean_sunrise = sunrise_str.split('.')[0] + '+00:00'
        clean_sunset = sunset_str.split('.')[0] + '+00:00'

        sunrise = datetime.fromisoformat(clean_sunrise)
        sunset = datetime.fromisoformat(clean_sunset)

        now = datetime.now(timezone.utc)

        return sunrise < now < sunset

    except ValueError as e:
        print(f"Time parsing error: {e}")
        return True

if __name__ == '__main__':
    app.run()
