﻿#include <Siv3D.hpp>


struct DetecterMsgFrame {
	int enemyX;
	int enemyY;
	int curDeltaX;
	int curDeltaY;
};

enum DetecterRes
{
	Safe = 0,
	Cheet,
	Error,
	Undecied
};

class DetecterClient {
private:
	static const int buffsize = 100;
	int buff[buffsize * 4] = { 0 };
	int ibuff;
	char ret[4] = {};
	TCPClient client;
	bool connected = false;
public:
	int port, fps;
	IPv4Address ip;

	DetecterClient() :ip("127.0.0.1"), port(50000), fps(240) {
		this->ibuff = 0;
		this->client.connect(this->ip, this->port);
	};

	DetecterRes push(DetecterMsgFrame frame) {
		if (this->client.isConnected())
		{
			if (not this->connected) // 接続された
			{
				connected = true;

				//Window::SetTitle(U"TCPClient: Connection established!");
			}


			this->buff[ibuff * 4 + 0] = frame.enemyX;
			this->buff[ibuff * 4 + 1] = frame.enemyY;
			this->buff[ibuff * 4 + 2] = frame.curDeltaX;
			this->buff[ibuff * 4 + 3] = frame.curDeltaY;
			this->ibuff++;

			if (this->ibuff == this->buffsize) {
				// 
				this->client.send(buff);

				// 受信
				while (client.read(ret));
				Console.open();
				std::cout << "data = " << ret[0] << ret[1] << ret[2] << ret[3] << std::endl;
				this->ibuff = 0;

				if (ret[0] != '0') {
					std::cout << "cheet detected !!" << std::endl;
					return DetecterRes::Cheet;
				}
				else {
					return DetecterRes::Safe;
				}
			}
			return DetecterRes::Undecied;

		}

		if (this->client.hasError()) // 切断/接続エラー
		{
			this->client.disconnect();

			this->connected = false;

			this->client.connect(this->ip, this->port);

			return DetecterRes::Error;
		}

		return DetecterRes::Undecied;
	}

	bool clear() {
		this->ibuff = 0;
		return true;
	}

	bool disconect() {
		if (this->connected) {
			this->client.disconnect();
			this->connected = false;
			return true;
		}
		else {
			return false;
		}
	}
};


struct Enemy
{
	const double speed = 0.2;
	const int maxCountSameDirectionContinue = 10000*1.5;
	const int maxCountContinueNoJump = 10000*2;
	const double yspeed = 1;
	const int maxCountContinueJump = 120/yspeed;

	const int maxX = 30;
	const int maxY = 10;
	//const double ycicle = 0.5;

	const std::vector<double> jumpPositionY;

	Vec3 position = { 0, 0, 30 };
	Cylinder shape = { position, 2, 4 };

	int direction = 1;
	int unsigned countContinueSameDirection = 0;
	int unsigned countContinueNoJump = 0;
	int unsigned countContinueJump = 0;
	std::random_device rng;

	void updatePosition()
	{
		// move X
		if (
			(position.x > maxX ||
			position.x < -maxX )
			||(
			rng() % maxCountSameDirectionContinue < countContinueSameDirection)
			&& countContinueJump == 0
			)
		{
			direction *= -1;
			countContinueSameDirection = 0;
		}
		else
		{
			countContinueSameDirection++;
		}
		position.x += speed * direction;

		// move Y
		if (countContinueJump > 0)
		{
			position.y = std::sin(countContinueJump * 3.14 / maxCountContinueJump) * maxY;
			if (countContinueJump  > (maxCountContinueJump))
			{
				countContinueJump = 0;
			}
			else {
				countContinueJump++;
			}
		}
		else
		{
			if (rng() % maxCountContinueNoJump < countContinueNoJump)
			{
				countContinueJump++;
				countContinueNoJump = 0;
			}
			else
			{
				countContinueNoJump++;
			}
		}
		shape = { position, 2, 8 };
	}
};




Vec3 rotateX(Vec3 vec3, double theta) {
	double y = vec3.y * Cos(theta) - vec3.z * Sin(theta);
	double z = vec3.y * Sin(theta) + vec3.z * Cos(theta);
	Vec3 result = { vec3.x, y, z };
	return result;
}

Vec3 rotateY(Vec3 vec3, double theta) {
	double x = vec3.x * Cos(theta) - vec3.z * Sin(theta);
	double z = vec3.x * Sin(theta) + vec3.z * Cos(theta);
	Vec3 result = { x, vec3.y, z };
	return result;
}

double abs2(double x, double y) {
	return Sqrt(x * x + y * y);
}

Vec2 cheat(Vec3 focusPosition, Vec3 enemyPosition, double HSensitivity, double VSensitivity, double correctRate = 0.1) {
	double deltaX = -Acos((focusPosition.x * enemyPosition.x + focusPosition.z * enemyPosition.z) / (abs2(focusPosition.x, focusPosition.z) * abs2(enemyPosition.x, enemyPosition.z))) / HSensitivity;
	if (focusPosition.x * enemyPosition.z - focusPosition.z * enemyPosition.x < 0)
	{
		deltaX = -deltaX;
	}
	double focusVAngle = Atan2(focusPosition.y, abs2(focusPosition.x, focusPosition.z));
	double enemyVAngle = Atan2(enemyPosition.y, abs2(enemyPosition.x, enemyPosition.z));
	double deltaY = -(enemyVAngle - focusVAngle) / VSensitivity;
	return Vec2{ deltaX * correctRate, deltaY * correctRate };
}


Vec2 enemyPositionToMouse(Vec3 focusPosition, Vec3 enemyPosition, double HSensitivity, double VSensitivity) {
	const double correctRate = 0.1;
	double deltaX = -Acos((focusPosition.x * enemyPosition.x + focusPosition.z * enemyPosition.z) / (abs2(focusPosition.x, focusPosition.z) * abs2(enemyPosition.x, enemyPosition.z))) / HSensitivity;
	if (focusPosition.x * enemyPosition.z - focusPosition.z * enemyPosition.x < 0)
	{
		deltaX = -deltaX;
	}
	double focusVAngle = Atan2(focusPosition.y, abs2(focusPosition.x, focusPosition.z));
	double enemyVAngle = Atan2(enemyPosition.y, abs2(enemyPosition.x, enemyPosition.z));
	double deltaY = -(enemyVAngle - focusVAngle) / VSensitivity;
	return Vec2{ deltaX, -deltaY };
}

Vec2 enemyPositionToScreen(double FOVH, Vec3 focusPosition, Vec3 enemyPosition) {
	double enemyHAngle = - Acos((focusPosition.x * enemyPosition.x + focusPosition.z * enemyPosition.z) / (abs2(focusPosition.x, focusPosition.z) * abs2(enemyPosition.x, enemyPosition.z)));
	if (focusPosition.x * enemyPosition.z - focusPosition.z * enemyPosition.x < 0)
	{
		enemyHAngle = -enemyHAngle;
	}

	double focusVAngle = Atan2(focusPosition.y, abs2(focusPosition.x, focusPosition.z));
	double enemyVAngle = Atan2(enemyPosition.y, abs2(enemyPosition.x, enemyPosition.z));
	enemyVAngle = enemyVAngle - focusVAngle;
	Vec2 enemyPos;
	Size windowSize = Scene::Size();
	enemyPos.x = (enemyHAngle / FOVH * (windowSize.x / 2));
	enemyPos.y = (enemyVAngle / FOVH * (9/16) * (windowSize.y / 2));
	return enemyPos;
}


struct Detector
{
	DetecterClient client;
	bool cheeted = false;

	bool detect(Vec2 cursorDelta, Vec2 enemyPosition, bool shooting = true)
	{
		if (shooting) {
			DetecterMsgFrame frame{ enemyPosition.x,enemyPosition.y,cursorDelta.x,cursorDelta.y };
			auto res = client.push(frame);
			switch (res)
			{
			case DetecterRes::Safe:
				cheeted = false;
				break;
			case DetecterRes::Cheet:
				cheeted = true;
				break;
			case DetecterRes::Error:
				break;
			case DetecterRes::Undecied:
				break;
			default:
				break;
			}
			return cheeted;
		}
		else {
			client.clear();
		}

		return false;
	}
};

void Main()
{
	Window::SetFullscreen(true);

	const ColorF backgroundColor = ColorF{ 0.4, 0.6, 0.8 }.removeSRGBCurve();
	const Texture uvChecker{ U"example/texture/uv.png", TextureDesc::MippedSRGB };
	const MSRenderTexture renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };

	const double HFOV_deg = 104;
	const double VFOV_deg = HFOV_deg *9/16;
	const double HFOV = Math::ToRadians(HFOV_deg);
	const double VFOV = Math::ToRadians(VFOV_deg);

	const double VSensitivity = 0.0005;
	const double HSensitivity = 0.0005;
	bool isCheatON = false;
	const double correctRate = 0.1;
	Detector detector;
	Font font{ 50 };

	Vec3 eyePosition{ 0, 0, 0 };
	Vec3 focusPosition{ 0, 0, 1 };

	Vec2 cursorDelta = Cursor::Delta();

	BasicCamera3D camera{ renderTexture.size(), VFOV, eyePosition, focusPosition };

	Cursor::SetDefaultStyle(CursorStyle::Hidden);

	Enemy enemy1;

	CSV csv;
	csv.writeRow(U"cheatCorrectRate");
	csv.writeRow(correctRate);
	csv.writeRow(
		U"Time",
		U"enemyScreenPositionX",
		U"enemyScreenPositionY",
		U"enemyMousePositionX",
		U"enemyMousePositionY",
		U"isCheatON",
		U"shooting",
		U"hitting",
		U"cursorDelataX",
		U"cursorDeltaY"
	);


	while (System::Update())
	{
		auto sSize = Scene::Size();
		const int windowSizeX = sSize.x;
		const int windowSizeY = sSize.y;
		
		if (KeyC.down()) {
			isCheatON = !isCheatON;
		}

		cursorDelta = Cursor::DeltaF();
		Cursor::SetPos(Scene::Center());
		Vec2 enemyScreenPosition = enemyPositionToScreen(HFOV, focusPosition, enemy1.position);
		Vec2 enemyMousePosition = enemyPositionToMouse(focusPosition, enemy1.position, HSensitivity, VSensitivity);

		if (isCheatON && MouseL.pressed())
		{
			cursorDelta += cheat(focusPosition, enemy1.position, HSensitivity, VSensitivity, correctRate);
		}

		double VAngle = Atan2(-focusPosition.y, abs2(focusPosition.x, focusPosition.z));
		double HAngle = Atan2(focusPosition.z, focusPosition.x);
		double nextVAngle = Clamp(cursorDelta.y * VSensitivity + VAngle, -Math::Pi, Math::Pi);
		double nextHAngle = -cursorDelta.x * HSensitivity + HAngle - Math::Pi / 2;
		focusPosition = rotateX(Vec3(0, 0, 1), nextVAngle);
		focusPosition = rotateY(focusPosition, nextHAngle);

		enemy1.updatePosition();

		Ray ray(eyePosition, focusPosition);
		bool isAimingToEnemy = ray.intersects(enemy1.shape) != none;
		const DateTime now = DateTime::Now();
		csv.writeRow(
			U"{:0>4}{:0>2}{:0>2}{:0>2}{:0>2}{:0>2}{:0>2}"_fmt(now.year, now.month, now.day, now.hour, now.minute, now.second, now.milliseconds),
			enemyScreenPosition.x,
			enemyScreenPosition.y,
			enemyMousePosition.x,
			enemyMousePosition.y,
			isCheatON && MouseL.pressed(),
			MouseL.pressed(),
			isAimingToEnemy,
			cursorDelta.x,
			cursorDelta.y
		);

		ClearPrint();
		Print << U"Press C to cheat.";
		Print << U"{:0>4}{:0>2}{:0>2}{:0>2}{:0>2}{:0>2}{:0>2}"_fmt(now.year, now.month, now.day, now.hour, now.minute, now.second, now.milliseconds),
		Print << U"ScreenPosition:{}"_fmt(enemyScreenPosition);
		Print << U"MousePosition:{}"_fmt(enemyMousePosition);
		Print << U"cheatON:{}"_fmt(isCheatON && MouseL.pressed());
		Print << U"shooting:{}"_fmt(MouseL.pressed());
		Print << U"aimingToEnemy:{}"_fmt(isAimingToEnemy);
		Print << U"cursorDelta{}"_fmt(cursorDelta);
		Print << U"FPS:{}"_fmt(Profiler::FPS());

		// 位置・注目点情報を更新
		camera.setView(eyePosition, focusPosition);
		//camera.setProjection(renderTexture.size(), VFOV);

		Graphics3D::SetCameraTransform(camera);

		auto center = Scene::Center();

		// 3D 描画
		{
			const ScopedRenderTarget3D target{ renderTexture.clear(backgroundColor) };
			Plane{ Vec3{ 0, -10, 0}, 100 }.draw();
			enemy1.shape.draw(ColorF{ 0.6, 0.4, 0.8 }.removeSRGBCurve());
		}

		// 3D シーンを 2D シーンに描画
		{
			Graphics3D::Flush();
			renderTexture.resolve();
			Shader::LinearToScreen(renderTexture);


			if (isAimingToEnemy)
			{
				Circle{ center.x,center.y, 5 }.draw(Color{ 0, 255, 0 });
			}
			else
			{
				Circle{ center.x,center.y, 5 }.draw(Color{ 255, 0, 0 });
			}
			if (isCheatON)
			{
				font(U"チートON").draw(windowSizeX / 2 - 50 * 2.5, 0, Palette::Red);
			}
			if (detector.detect(cursorDelta, enemyMousePosition, MouseL.pressed()))
			{
				font(U"DETECTED!!").drawAt(Scene::Center(), Palette::Red);
			}
		}
	}
	const DateTime t = DateTime::Now();
	csv.save(U"log/log{:0>2}{:0>2}_{:0>2}{:0>2}{:0>2}.csv"_fmt(t.month, t.day, t.hour, t.minute, t.second));
}
