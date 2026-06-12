# Lab GCS MVP

Lab GCS MVP là ứng dụng trạm điều khiển mặt đất (Ground Control Station - GCS)
cho UAV/drone, được xây dựng bằng Qt 6, QML, C++17, CMake và QtTest.

Ứng dụng này tập trung vào các chức năng cốt lõi của một GCS trong môi trường lab:

- Lập kế hoạch bay trên bản đồ.
- Tạo, sửa, xóa và sắp xếp waypoint.
- Validate mission trước khi truyền.
- Upload/download mission với MockVehicle và SITL.
- Theo dõi telemetry: vị trí, độ cao, tốc độ, heading, roll/pitch, pin, GPS và heartbeat.
- Hiển thị bản đồ bay, marker drone và trail.
- Mô phỏng drone bay theo mission bằng MockVehicle.
- Điều khiển thủ công bằng mock joystick qua SafetyGate.
- Ghi Event Log và file JSONL.
- Chuẩn bị sẵn hướng kết nối serial/hardware read-only cho giai đoạn sau.

## Lưu Ý An Toàn

Phiên bản hiện tại được dùng cho nghiên cứu, demo và mô phỏng trong lab. Không dùng để
bay drone thật ngoài đời.

Trong chế độ serial/hardware, ứng dụng chỉ thiết kế theo hướng telemetry read-only.
Các lệnh outbound đến hardware thật như arm, takeoff, RTL, mission start, manual
control và mission write đều không phải phạm vi sử dụng của phiên bản này.

## Yêu Cầu Khi Cài Đặt

Khuyến nghị chạy trên Windows 10/11.

Cần cài các công cụ sau:

1. Git for Windows
2. Qt 6.4 trở lên, khuyến nghị Qt 6.x MinGW 64-bit
3. Qt modules:
   - Core
   - Gui
   - Network
   - Qml
   - Quick
   - QuickControls2
   - Test
   - Positioning
   - Location
   - SerialPort, tùy chọn nếu cần mở COM port read-only
4. CMake 3.21 trở lên
5. Ninja
6. MinGW C++ compiler tương thích với bản Qt đã cài

Có thể cài Qt, CMake, Ninja và MinGW bằng Qt Online Installer. Khi cài Qt, nên
chọn bộ `MinGW 64-bit` và các module `Qt Location`, `Qt Positioning`,
`Qt SerialPort`.

Nếu không cài `Qt SerialPort`, app vẫn build được nhưng chức năng SerialLink sẽ
chạy ở chế độ stub.

## Clone Repo

```powershell
git clone <REPO_URL> LabGCS
cd LabGCS
```

Nếu đã có repo sẵn:

```powershell
cd <duong-dan-den-repo>
```

## Cấu Hình Đường Dẫn Tool

Script build có thể nhận đường dẫn tool qua tham số hoặc biến môi trường.

Ví dụ với PowerShell:

```powershell
$env:GCS_QT_PATH = '<Qt-MinGW-64-bit-path>'
$env:GCS_CMAKE_PATH = '<cmake.exe-path>'
$env:GCS_MINGW_PATH = '<MinGW-root-path>'
$env:GCS_NINJA_PATH = '<ninja.exe-path>'
```

Thay các placeholder trên bằng đường dẫn thực tế trên máy đang build.

## Build Ứng Dụng

Cách khuyến nghị:

```powershell
.\scripts\build_windows.ps1
```

Nếu Windows chặn PowerShell script:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1
```

Nếu muốn truyền đường dẫn trực tiếp thay vì dùng biến môi trường:

```powershell
.\scripts\build_windows.ps1 `
  -QtPath '<Qt-MinGW-64-bit-path>' `
  -CMakePath '<cmake.exe-path>' `
  -MingwPath '<MinGW-root-path>' `
  -NinjaPath '<ninja.exe-path>'
```

Build thành công sẽ tạo file:

```text
build\LabGCSApp.exe
```

## Chạy Ứng Dụng

Nếu đã build xong:

```powershell
.\scripts\run_windows.ps1 -SkipBuild
```

Nếu muốn script tự build trước khi chạy:

```powershell
.\scripts\run_windows.ps1
```

Cách chạy thủ công:

```powershell
.\build\LabGCSApp.exe
```

Khi mở app, màn hình đầu tiên có hai lựa chọn:

- `Mock`: chạy với xe bay mô phỏng nội bộ, không cần cài PX4/ArduPilot.
- `SITL or hardware`: mở màn hình kết nối UDP/Serial để dùng với SITL hoặc
  telemetry serial read-only.

## Chạy Nhanh Bằng MockVehicle

Đây là cách đơn giản nhất để mở và sử dụng app sau khi build:

1. Mở LabGCS.
2. Chọn `Mock`.
3. Vào tab `Fly` để xem telemetry mô phỏng và bản đồ tracking.
4. Vào tab `Plan`.
5. Double-click trên bản đồ để tạo waypoint.
6. Bấm `Validate` để kiểm tra mission.
7. Bấm `Simulate` để xem drone mô phỏng đi theo các waypoint.
8. Bật `Show trail` trong tab `Fly` nếu muốn xem đường bay.
9. Mở rộng `Event Log` nếu cần xem log chi tiết.

Chế độ Mock không cần WSL, PX4, ArduPilot hay flight controller thật.

## Kết Nối SITL Tùy Chọn

Nếu muốn chạy với PX4 SITL hoặc ArduPilot SITL, hãy chạy simulator bên ngoài trước,
sau đó mở UDP listener trong LabGCS.

Trong LabGCS:

1. Chọn `SITL or hardware`.
2. Chọn mode `UDP`.
3. Listen host: `0.0.0.0`.
4. Listen port: `14550`.
5. Bấm `Start UDP listener`.

Simulator cần gửi MAVLink UDP về máy đang chạy LabGCS tại cổng `14550`.

Hướng dẫn chi tiết cho SITL nằm trong:

- `docs/sitl_testing.md`
- `tools/sitl/README.md`

README này chỉ tập trung vào việc cài, build và chạy ứng dụng.

## Manual Control

Tab `Manual` dùng mock joystick để tạo đầu vào điều khiển. Luồng điều khiển đi
qua SafetyGate trước khi đến sink.

Quy trình sử dụng cơ bản:

1. Chạy app với `Mock` hoặc kết nối UDP SITL.
2. Vào tab `Manual`.
3. Bấm `Connect mock joystick`.
4. Bấm `Enable`.
5. Điều chỉnh các slider `Pitch`, `Roll`, `Throttle`, `Yaw`.

Nếu thiếu điều kiện an toàn như không có vehicle, heartbeat stale, joystick ngắt
kết nối hoặc sink không hợp lệ, SafetyGate sẽ chuyển trạng thái sang `Blocked`
hoặc `Failsafe`.

## Log

Ứng dụng hiển thị log tại panel `Event Log` và ghi file JSONL trong thư mục dữ
liệu người dùng của hệ điều hành.

Trên Windows, đường dẫn thường có dạng:

```text
%LOCALAPPDATA%\UAV Lab\LabGCS\logs\labgcs-YYYYMMDD-HHMMSS.jsonl
```

Trong UI có các nút:

- `Open folder`: mở thư mục chứa log.
- `Copy path`: copy đường dẫn file log.
- Icon expand trong `Event Log`: mở cửa sổ log lớn hơn.

## Chạy Test Dành Cho Developer

Nếu cần chạy unit test:

```powershell
.\scripts\test_windows.ps1
```

Nếu đã build xong:

```powershell
.\scripts\test_windows.ps1 -SkipBuild
```

## Tài Liệu Trong Repo

- `docs/architecture.md`: kiến trúc tổng quan.
- `docs/safety.md`: nguyên tắc an toàn.
- `docs/current_capabilities.md`: các khả năng hiện có.
- `docs/sitl_testing.md`: ghi chú chạy với SITL.
- `docs/hitl_real_uav_preparation.md`: chuẩn bị cho HITL/real UAV.
- `docs/development_plan.md`: kế hoạch phát triển.
- `docs/limitations_and_future_work.md`: giới hạn và hướng phát triển.
- `docs/safety_audit.md`: ghi chú audit an toàn.

## Troubleshooting

| Vấn đề | Cách xử lý |
|---|---|
| Build báo không tìm thấy Qt | Kiểm tra `-QtPath` hoặc `$env:GCS_QT_PATH`. Thư mục Qt phải có `bin\Qt6Core.dll`. |
| Build báo không tìm thấy CMake/Ninja/MinGW | Cài thêm trong Qt Maintenance Tool hoặc truyền đúng `-CMakePath`, `-NinjaPath`, `-MingwPath`. |
| Link lỗi `cannot open output file LabGCSApp.exe: Permission denied` | Đóng LabGCS đang chạy rồi build lại. |
| App mở nhưng map không tải tile | Kiểm tra kết nối mạng. Bản đồ dùng Qt Location/OpenStreetMap. |
| Không thấy SITL connected | Kiểm tra LabGCS đang listen port `14550` và simulator đang output MAVLink về đúng IP/cùng port. |
| Manual bị blocked | Kiểm tra đã connect mock joystick, đã bấm `Enable`, heartbeat còn mới và vehicle là Mock/SITL. |
| Serial không mở được | Cài Qt SerialPort và build lại; nếu không có module này, app sẽ dùng stub. |

## Phạm Vi Hiện Tại

LabGCS hiện phù hợp để demo, lập kế hoạch bay, theo dõi telemetry, mô phỏng mission
và thực hiện manual control trong môi trường Mock/SITL. Phiên bản này chưa phải
phần mềm bay drone thật ngoài đời.
