# include <Siv3D.hpp>

struct Enemy
{
	const double speed = 1;
	const unsigned int maxCountSameDirectionContinue = 10000;
	const unsigned int maxCountContinueNoJump = 10000;
	const unsigned int maxCountContinueJump = 20;

	const unsigned int maxX = 30;
	const unsigned int maxY = 10;

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
			position.x > maxX ||
			-position.x > maxX ||
			rng() % maxCountSameDirectionContinue < countContinueSameDirection
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
			if (countContinueJump > maxCountContinueJump)
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
		shape = { position, 2, 4 };
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

Vec2 cheat(Vec3 focusPosition, Vec3 enemyPosition, double HSensitivity, double VSensitivity) {
	const double correctRate = 0.1;
	double deltaX = - Acos((focusPosition.x * enemyPosition.x + focusPosition.z * enemyPosition.z) / (abs2(focusPosition.x, focusPosition.z) * abs2(enemyPosition.x, enemyPosition.z))) / HSensitivity;
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
	enemyPos.x = (enemyHAngle / (FOVH*Math::Pi/180)) * (windowSize.x / 2);
	enemyPos.y = (enemyVAngle / (FOVH*Math::Pi*16/180/9)) * (windowSize.y / 2);
	return enemyPos;
}


struct Detector
{

	bool detect(Vec2 cursorDelta, Vec3 enemyPosition)
	{
		//TODO
		return false;
	}
};

void Main()
{
	const int windowSizeX = 1280;
	const int windowSizeY = 720;
	Window::Resize(windowSizeX, windowSizeY);

	CSV csv;
	csv.writeRow(U"enemyScreenPositionX", U"enemyScreenPositionY", U"enemyMousePositionX", U"enemyMousePositionY", U"isCheatON", U"cursorDelataX", U"cursorDeltaY");

	const ColorF backgroundColor = ColorF{ 0.4, 0.6, 0.8 }.removeSRGBCurve();
	const Texture uvChecker{ U"example/texture/uv.png", TextureDesc::MippedSRGB };
	const MSRenderTexture renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };

	ColorF color2{ 1.0, 0.5, 0.0 };
	double FOVX = 70_deg;
	Vec2 FOV{ FOVX, FOVX*9/16 };
	double VSensitivity = 0.005;
	double HSensitivity = 0.005;
	bool isCheatON = false;
	Detector detector;
	Font font{ 50 };

	Vec3 eyePosition{ 0, 0, 0 };
	Vec3 focusPosition{ 0, 0, 1 };

	Vec2 cursorDelta = Cursor::Delta();

	BasicCamera3D camera{ renderTexture.size(), FOV.y, eyePosition, focusPosition };

	Cursor::SetDefaultStyle(CursorStyle::Hidden);

	Enemy enemy1;

	while (System::Update())
	{
		if (KeyC.down()) {
			isCheatON = !isCheatON;
		}

		cursorDelta = Cursor::DeltaF();
		Cursor::SetPos(Scene::Center());
		Vec2 enemyScreenPosition = enemyPositionToScreen(FOV.x, focusPosition, enemy1.position);
		Vec2 enemyMousePosition = enemyPositionToMouse(focusPosition, enemy1.position, HSensitivity, VSensitivity);

		if (isCheatON && MouseL.pressed())
		{
			cursorDelta += cheat(focusPosition, enemy1.position, HSensitivity, VSensitivity);
		}


		csv.writeRow(enemyScreenPosition.x, enemyScreenPosition.y, enemyMousePosition.x, enemyMousePosition.y, isCheatON && MouseL.pressed(), cursorDelta.x, cursorDelta.y);

		double VAngle = Atan2(-focusPosition.y, abs2(focusPosition.x, focusPosition.z));
		double HAngle = Atan2(focusPosition.z, focusPosition.x);
		double nextVAngle = Clamp(cursorDelta.y * VSensitivity + VAngle, -Math::Pi, Math::Pi);
		double nextHAngle = -cursorDelta.x * HSensitivity + HAngle - Math::Pi/2;
		focusPosition = rotateX(Vec3(0, 0, 1), nextVAngle);
		focusPosition = rotateY(focusPosition, nextHAngle);


		enemy1.updatePosition();

		ClearPrint();
		Print << U"Press C to cheat.";
		Print << U"ScreenPosition:{}"_fmt(enemyScreenPosition);
		Print << U"MousePosition:{}"_fmt(enemyMousePosition);

		// 位置・注目点情報を更新
		camera.setView(eyePosition, focusPosition);
		//camera.setProjection(renderTexture.size(), FOV * 3.14 / 120);

		Graphics3D::SetCameraTransform(camera);

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

			Ray ray(eyePosition, focusPosition);
			if (ray.intersects(enemy1.shape))
			{
				Circle{ windowSizeX/2, windowSizeY/2, 5 }.draw(Color{ 0, 255, 0 });
			}
			else
			{
				Circle{ windowSizeX/2, windowSizeY/2, 5 }.draw(Color{ 255, 0, 0 });
			}
			if (isCheatON)
			{
				font(U"チートON").draw(windowSizeX / 2 - 50 * 2.5, 0, Palette::Red);
			}
			if (detector.detect(cursorDelta, enemy1.position))
			{
				font(U"DETECTED!!").drawAt(Scene::Center(), Palette::Red);
			}
		}
	}
	const DateTime t = DateTime::Now();
	csv.save(U"log/log{:0>2}{:0>2}_{:0>2}{:0>2}{:0>2}.csv"_fmt(t.month, t.day, t.hour, t.minute, t.second));
}
