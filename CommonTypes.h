// CommonTypes.h - 通用类型定义
#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

//辐射噪声模型
#define Topic_RadiationNoise "topic_radiation_noise"
//目标运动模型
#define Topic_TargetMotion "topic_target_motion"
//线谱模型配置
#define Topic_LineSpectrum "topic_line_spectrum"
//声纳初始化参数
#define Topic_SonarInitData "topic_sonar_init_data"
// 传播损失参数
#define Topic_PropagationParams "topic_propagation_params"
//海洋噪声
#define Topic_OceanNoise "topic_ocean_noise"
// 目标初始化
#define Topic_InitTarget "topic_init_target"
// 鱼雷数据
#define Topic_TorpedoStepOutput "topic_torpedo_output"
// 鱼雷发射指令
#define Topic_TorpedoLaunch "topic_torpedo_launch"
// 目标位置
#define Topic_TrackingOutput "topic_tracking_output"

// 仿真步进
#define Topic_SimulationStep "topic_sim_step"
//环境参数
#define Topic_Environment "topic_environment"
//声纳阵列
#define Topic_SonarArray "topic_sonar_array"
//声纳状态
#define Topic_SonarState "topic_sonar_state"
//声纳处理
#define Topic_SonarConfig "topic_sonar_config"
//传播损失
#define Topic_PropagationLoss "topic_propagation_loss"
//声纳检测结果
#define Topic_DetectionResult "topic_detection_result"
//时域信号包
#define Topic_SignalPacket "topic_signal_packet"
// 频域谱图结构体
#define Topic_SpectrumData "topic_spectrum_data"
//目标运动状态
#define Topic_MotionState "topic_motion_state"
// 飞机状态
#define Topic_AircraftState "topic_aircraft_state"

// 主动声纳
#define Topic_ActiveSonar "topic_active_sonar"
// 主动声纳参数
#define Topic_ActiveSonarConfig "topic_active_sonar_config"

//-----------------------
#define Topic_AMTIMode "M_SOA_1A_25"

#include <vector>
#include <cmath>
#include <iostream>
#include <limits>
#define SOUND_SPEED 1500.0
// 常量定义
const double PI = 3.14159265358979323846;
const double SAMPLING_RATE = 2500.0; // 44.1kHz
const double REF_PRESSURE = 1.0;      // 1 uPa
const double EARTH_RADIUS = 6371000.0; // 地球半径(m)
const double INVALID_VALUE = std::numeric_limits<double>::quiet_NaN();
const double DEFAULT_DETECTION_THRESHOLD = 5.0; // 默认检测阈值
const double MIN_DETECTION_SNR = 0.0;          // 最小检测SNR
// 地理位置结构体
struct GeoPosition {
	double latitude;   // 纬度(度)
	double longitude;  // 经度(度)
	double vertical_position; // 海平面参考高度(m): 海面上为正，海面下为负

	GeoPosition(double lat = INVALID_VALUE, double lon = INVALID_VALUE, double d = INVALID_VALUE)
		: latitude(lat), longitude(lon), vertical_position(d) {
	}

	static GeoPosition fromAltitude(double lat, double lon, double altitude_m) {
		return GeoPosition(lat, lon, altitude_m);
	}

	static GeoPosition fromUnderwaterDepth(double lat, double lon, double depth_below_surface_m) {
		return GeoPosition(lat, lon, -std::abs(depth_below_surface_m));
	}

	double altitude() const {
		return vertical_position;
	}

	double underwaterDepth() const {
		return vertical_position < 0.0 ? -vertical_position : 0.0;
	}
	// 检查位置是否有效
	bool isValid() const {
		return !std::isnan(latitude) && !std::isnan(longitude) && !std::isnan(vertical_position);
	}
};

enum SignalSourceType
{
	Line = 0,		// 线谱噪声
	Ocean,			// 环境噪声
	Radiation,		// 辐射噪声
	Source			// 源
};

// 通用结构体：时域信号包
struct SignalPacket {
	std::vector<double> time_domain_data; // 归一化幅值
	double timestamp = 0.0;
	double duration = 0.0;
	GeoPosition source_position; // 信号源位置
};

// 频域谱图结构体
struct SpectrumData {
	std::vector<double> frequencies;         // 频率轴(Hz)
	std::vector<double> magnitudes;          // 频谱幅值(线性)
	std::vector<double> line_spectra_freqs;  // 线谱频率
	std::vector<double> line_spectra_levels; // 线谱级
	double total_spl; // 总声压级
};

// 目标运动状态
struct MotionState {
	GeoPosition position;     // 位置(纬度, 经度, 海平面参考高度)
	double speed = 0.0;       // 航速(knots)
	double heading = 0.0;     // 航向(度，正北为0，顺时针)

	MotionState() : position(0, 0, 0) {}
	MotionState(double lat, double lon, double d, double s = 0.0, double h = 0.0)
		: position(lat, lon, d), speed(s), heading(h) {
	}
};

// 环境参数
struct EnvironmentParams {
	double temperature = 15.0;    // 温度(℃)
	double salinity = 35.0;       // 盐度(‰)
	double depth = 1000.0;        // 水深(m)
	int sea_state = 2;            // 海况等级(0-9)
	double shipping_density = 0.5; // 航运密度(艘/100km²)
};

// 声纳检测结果
struct DetectionResult {
	double measured_bearing = INVALID_VALUE;   // 测量方位角(度)
	double true_bearing = INVALID_VALUE;       // 真实方位角(度)
	double bearing_error = INVALID_VALUE;      // 方位角误差(度)
	double snr = INVALID_VALUE;                // 信噪比(dB)
	double distance = INVALID_VALUE;           // 距离(m)
	bool is_detected = false;                  // 是否检测到
	GeoPosition target_position;               // 目标位置估计
	double confidence = 0.0;                   // 检测置信度(0-1)
	DetectionResult() = default;

	// 初始化一个有效的检测结果
	void initializeDetection(double measured, double true_bearing_val,
		double snr_val, double dist,
		const GeoPosition& target_pos, double conf) {
		measured_bearing = measured;
		true_bearing = true_bearing_val;
		bearing_error = measured - true_bearing_val;
		// 调整误差范围到[-180, 180]
		if (bearing_error > 180.0) bearing_error -= 360.0;
		if (bearing_error < -180.0) bearing_error += 360.0;
		snr = snr_val;
		distance = dist;
		target_position = target_pos;
		confidence = conf;
		is_detected = true;
	}

	// 初始化一个未检测到的结果（只有真实方位和距离）
	void initializeNoDetection(double true_bearing_val, double dist) {
		true_bearing = true_bearing_val;
		distance = dist;
		measured_bearing = INVALID_VALUE;
		bearing_error = INVALID_VALUE;
		snr = INVALID_VALUE;
		target_position = GeoPosition();
		confidence = 0.0;
		is_detected = false;
	}

	// 检查值是否有效
	bool isValueValid(double value) const {
		return !std::isnan(value) && std::isfinite(value);
	}
};

// 声纳跟踪信息
struct TrackInfo {
	int id;                                        // 跟踪ID
	DetectionResult latest_detection;              // 最新检测结果
	std::vector<DetectionResult> detection_history;// 检测历史
	bool is_active;
};

//----6
enum class SonarBuoyState {
	PLANNED = 0,               //未投放
	FREE_FALL = 1,             //自由落体
	CHUTE_DESCENT = 2,         //开伞减速
	SPLASHED_WAITING = 3,      //已落水，等待稳定
	ACTIVE = 4                 //工作中
};

namespace GeoUtils {
	// 经纬度转换为米（墨卡托投影简化版）
	inline void latLonToMeters(double lat, double lon, double& x, double& y) {
		// 简化墨卡托投影
		double lat_rad = lat * PI / 180.0;
		double lon_rad = lon * PI / 180.0;

		// 基准纬度（取第一个点的纬度）
		static double ref_lat = 0.0;
		if (ref_lat == 0.0) ref_lat = lat_rad;

		x = EARTH_RADIUS * lon_rad * cos(ref_lat);
		y = EARTH_RADIUS * lat_rad;
	}

	inline void relativeMetersFromReference(const GeoPosition& ref, const GeoPosition& pos, double& x, double& y) {
		double d_lat = (pos.latitude - ref.latitude) * PI / 180.0;
		double d_lon = (pos.longitude - ref.longitude) * PI / 180.0;
		double ref_lat_rad = ref.latitude * PI / 180.0;

		y = d_lat * EARTH_RADIUS;
		x = d_lon * EARTH_RADIUS * std::cos(ref_lat_rad);
	}

	// 计算两点间的距离（球面距离）
	inline double calculateDistance(const GeoPosition& pos1, const GeoPosition& pos2) {
		// 检查输入有效性
		if (!pos1.isValid() || !pos2.isValid()) {
			std::cerr << "[GeoUtils] Warning: Invalid position in calculateDistance" << std::endl;
			return 0.0;
		}

		double lat1 = pos1.latitude * PI / 180.0;
		double lon1 = pos1.longitude * PI / 180.0;
		double lat2 = pos2.latitude * PI / 180.0;
		double lon2 = pos2.longitude * PI / 180.0;

		// 检查角度是否在合理范围内
		if (std::abs(lat1) > PI / 2 || std::abs(lat2) > PI / 2) {
			std::cerr << "[GeoUtils] Error: Latitude out of range" << std::endl;
			return 0.0;
		}

		double dlat = lat2 - lat1;
		double dlon = lon2 - lon1;

		double a = sin(dlat / 2) * sin(dlat / 2) +
			cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);

		// 防止数值误差导致a大于1
		if (a > 1.0) a = 1.0;
		if (a < 0.0) a = 0.0;

		double c = 2 * atan2(sqrt(a), sqrt(1 - a));

		double horizontal_distance = EARTH_RADIUS * c;
		double vertical_difference = fabs(pos1.vertical_position - pos2.vertical_position);

		// 防止除零和无效计算
		if (!std::isfinite(horizontal_distance) || !std::isfinite(vertical_difference)) {
			std::cerr << "[GeoUtils] Error: Invalid distance calculation" << std::endl;
			return 0.0;
		}

		// 总距离（三维欧氏距离）
		return sqrt(horizontal_distance * horizontal_distance +
			vertical_difference * vertical_difference);
	}

	// 计算方位角（从pos1到pos2，正北为0，顺时针）
	inline double calculateBearing(const GeoPosition& pos1, const GeoPosition& pos2) {
		// 检查输入有效性
		if (!pos1.isValid() || !pos2.isValid()) {
			std::cerr << "[GeoUtils] Warning: Invalid position in calculateBearing" << std::endl;
			return 0.0;
		}

		// 如果位置相同，返回0度
		if (pos1.latitude == pos2.latitude && pos1.longitude == pos2.longitude) {
			return 0.0;
		}

		double lat1 = pos1.latitude * PI / 180.0;
		double lon1 = pos1.longitude * PI / 180.0;
		double lat2 = pos2.latitude * PI / 180.0;
		double lon2 = pos2.longitude * PI / 180.0;

		double dlon = lon2 - lon1;

		double y = sin(dlon) * cos(lat2);
		double x = cos(lat1) * sin(lat2) -
			sin(lat1) * cos(lat2) * cos(dlon);

		// 防止除零
		if (fabs(x) < 1e-10 && fabs(y) < 1e-10) {
			return 0.0;
		}

		double bearing = atan2(y, x) * 180.0 / PI;

		// 转换为0-360度
		bearing = fmod((bearing + 360.0), 360.0);

		// 检查结果是否有效
		if (!std::isfinite(bearing) || bearing < 0 || bearing > 360) {
			std::cerr << "[GeoUtils] Error: Invalid bearing calculated: " << bearing << std::endl;
			return 0.0;
		}

		return bearing;
	}

	// 根据方位角和距离计算新位置
	inline GeoPosition calculateNewPosition(const GeoPosition& start,
		double bearing_deg,
		double distance_m,
		double delta_vertical_position = 0.0) {
		double bearing_rad = bearing_deg * PI / 180.0;
		double lat1 = start.latitude * PI / 180.0;
		double lon1 = start.longitude * PI / 180.0;

		// 地球半径
		double R = EARTH_RADIUS;

		// 计算新的纬度
		double lat2 = asin(sin(lat1) * cos(distance_m / R) +
			cos(lat1) * sin(distance_m / R) * cos(bearing_rad));

		// 计算新的经度
		double lon2 = lon1 + atan2(sin(bearing_rad) * sin(distance_m / R) * cos(lat1),
			cos(distance_m / R) - sin(lat1) * sin(lat2));

		GeoPosition new_pos;
		new_pos.latitude = lat2 * 180.0 / PI;
		new_pos.longitude = lon2 * 180.0 / PI;
		new_pos.vertical_position = start.vertical_position + delta_vertical_position;

		return new_pos;
	}

	inline double calculateDopplerFactor(const GeoPosition& src_pos, double src_speed_knots, double src_heading, const GeoPosition& rx_pos) {
		// 1. 计算从源到接收者的方位角 (视线方向 LOS)
		double bearing_to_rx = calculateBearing(src_pos, rx_pos);

		// 2. 计算航向与视线方向的夹角
		// 夹角越小，说明目标正对着接收者冲过来
		double angle_diff_rad = (src_heading - bearing_to_rx) * PI / 180.0;

		// 3. 计算径向速度 (Radial Velocity)
		double speed_mps = src_speed_knots * 0.514444;
		double v_radial = speed_mps * cos(angle_diff_rad);

		// 4. 多普勒公式
		// v_radial 为正表示靠近，分母变小，频率升高
		if (std::abs(SOUND_SPEED - v_radial) < 0.1) return 1.0;
		return SOUND_SPEED / (SOUND_SPEED - v_radial);
	}

	// 新增：局部平面投影逆变换 (米 -> 经纬度)
	// ref_lat: 参考点纬度(弧度)
	inline GeoPosition metersToLatLon(double x, double y, const GeoPosition& ref_pos) {
		double ref_lat_rad = ref_pos.latitude * PI / 180.0;
		double ref_lon_rad = ref_pos.longitude * PI / 180.0;

		double lat_rad = y / EARTH_RADIUS + ref_lat_rad;
		double lon_rad = x / (EARTH_RADIUS * cos(ref_lat_rad)) + ref_lon_rad;

		return GeoPosition(lat_rad * 180.0 / PI, lon_rad * 180.0 / PI, ref_pos.vertical_position);
	}

	// 新增：计算两条射线的交点 (三角定位核心算法)
	// pos1/bearing1: 声纳1的位置和测得的方位角
	// pos2/bearing2: 声纳2的位置和测得的方位角
	inline GeoPosition calculateIntersection(const GeoPosition& pos1, double bearing1,
		const GeoPosition& pos2, double bearing2) {
		// 1. 将经纬度转换为局部坐标 (以pos1为原点 (0,0))
		double x1 = 0.0, y1 = 0.0;
		double x2, y2;

		// 计算pos2相对于pos1的偏移
		// 简单的平坦地球近似，适用于短距离(<100km)
		double d_lat = (pos2.latitude - pos1.latitude) * PI / 180.0;
		double d_lon = (pos2.longitude - pos1.longitude) * PI / 180.0;
		double lat_mean = (pos1.latitude + pos2.latitude) / 2.0 * PI / 180.0;

		x2 = d_lon * EARTH_RADIUS * cos(lat_mean);
		y2 = d_lat * EARTH_RADIUS;

		// 2. 将方位角(正北为0，顺时针)转换为数学角度(正东为0，逆时针)
		// Math Angle = 90 - Bearing
		double theta1 = (90.0 - bearing1) * PI / 180.0;
		double theta2 = (90.0 - bearing2) * PI / 180.0;

		// 3. 直线方程: y - y0 = k * (x - x0) => kx - y + (y0 - kx0) = 0
		// k = tan(theta)
		// 注意处理垂直线 (theta = 90 or 270)

		const double EPSILON = 1e-9;
		double tan1 = std::tan(theta1);
		double tan2 = std::tan(theta2);

		// 如果两条线平行，无法定位
		if (std::abs(tan1 - tan2) < 1e-4) return GeoPosition(INVALID_VALUE, INVALID_VALUE, INVALID_VALUE);

		// 解方程组:
		// y = tan1 * x  (因为x1=0, y1=0)
		// y - y2 = tan2 * (x - x2)

		// tan1 * x = tan2 * x - tan2 * x2 + y2
		// x * (tan1 - tan2) = y2 - tan2 * x2
		double x_target = (y2 - x2 * tan2) / (tan1 - tan2);
		double y_target = tan1 * x_target;

		// 4. 转回经纬度
		return metersToLatLon(x_target, y_target, pos1);
	}
}

#endif // COMMON_TYPES_H
