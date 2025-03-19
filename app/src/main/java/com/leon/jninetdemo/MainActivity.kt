package com.leon.jninetdemo

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Card
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.google.gson.Gson
import com.leon.jninetdemo.data.Forecast
import com.leon.jninetdemo.data.WeatherResponse
import com.leon.jninetdemo.ui.theme.JniNetDemoTheme

class MainActivity : ComponentActivity() {
    companion object {
        init {
            System.loadLibrary("NetLib") // 加载 native 库
        }
    }

    private external fun getNetDataFromJNI(): String // 声明 native 方法

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // 设置 Compose 界面
        setContent {
            JniNetDemoTheme {
                // Surface 用于设置背景色等
                Surface(color = MaterialTheme.colorScheme.background) {
                    WeatherScreen(weatherResponse = jsonToWeatherResponse(getNetDataFromJNI()))
                }
            }
        }

    }
}

@Composable
fun WeatherScreen(weatherResponse: WeatherResponse) {
    val forecastList = weatherResponse.data.forecast

    Column(modifier = Modifier.padding(16.dp)) {
        Text(
            text = "All data from JNI",
            style = MaterialTheme.typography.titleLarge,
            modifier = Modifier
                .align(Alignment.CenterHorizontally) // 居中对齐
                .padding(bottom = 8.dp) // 给标题加一些底部间距
        )
        Text(
            text = "城市: ${weatherResponse.cityInfo.city}",
            style = MaterialTheme.typography.titleLarge
        )
        Text(text = "日期: ${weatherResponse.date}", style = MaterialTheme.typography.bodyMedium)
        Spacer(modifier = Modifier.height(8.dp))

        Text(
            text = "当前温度: ${weatherResponse.data.wendu}℃",
            style = MaterialTheme.typography.headlineMedium
        )
        Text(
            text = "空气质量: ${weatherResponse.data.quality}",
            style = MaterialTheme.typography.bodyMedium
        )
        Text(
            text = "湿度: ${weatherResponse.data.shidu}",
            style = MaterialTheme.typography.bodyMedium
        )
        Text(
            text = "PM25: ${weatherResponse.data.pm25}",
            style = MaterialTheme.typography.bodyMedium
        )
        Text(
            text = "PM10: ${weatherResponse.data.pm10}",
            style = MaterialTheme.typography.bodyMedium
        )
        Text(
            text = "感冒提示: ${weatherResponse.data.ganmao}",
            style = MaterialTheme.typography.bodyMedium
        )
        Spacer(modifier = Modifier.height(16.dp))

        // 展示天气预报
        LazyColumn {
            items(forecastList) { forecast ->
                WeatherForecastItem(forecast)
            }
        }
    }
}

@Composable
fun WeatherForecastItem(forecast: Forecast) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(8.dp)
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(
                text = "日期: ${forecast.date} (${forecast.week})",
                style = MaterialTheme.typography.titleMedium
            )
            Text(text = "天气: ${forecast.type}", style = MaterialTheme.typography.bodyMedium)
            Text(
                text = "气温: ${forecast.high} / ${forecast.low}",
                style = MaterialTheme.typography.bodyLarge
            )
            Text(
                text = "空气质量指数(AQI): ${forecast.aqi}",
                style = MaterialTheme.typography.bodyMedium
            )
            Text(
                text = "风速: ${forecast.fx} ${forecast.fl}",
                style = MaterialTheme.typography.bodyMedium
            )
            Text(text = "提示: ${forecast.notice}", style = MaterialTheme.typography.bodyMedium)
        }
    }
}

fun jsonToWeatherResponse(jsonString: String): WeatherResponse {
    val gson = Gson()
    return gson.fromJson(jsonString, WeatherResponse::class.java)
}


