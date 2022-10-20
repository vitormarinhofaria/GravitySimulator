// GravitySim.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "Renderer.h"
#include "Quad2D.h"
#include "ConstBuffer.h"
#include "SingleQuad.h"
#include "Utils.h"

std::unordered_map<int, int> keyStates;

void handleKey(SDL_Keycode key, int state) {
	keyStates[key] = state;
	switch (key) {
	case SDLK_ESCAPE: {
		SDL_Event ev{};
		ev.type = SDL_QUIT;
		SDL_PushEvent(&ev);
		break;
	}
	default:
		break;
	}
}

int WINDOW_WIDTH = 1800;
int WINDOW_HEIGHT = 900;

int main(int argc, char** argv)
{
	auto result = SDL_Init(SDL_INIT_EVERYTHING);
	if (result != 0)
		std::cout << "Failed to initialize SDL\n";

	SDL_Window* window = SDL_CreateWindow("GravitySim", 50, 50, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

	SDL_SysWMinfo window_info{};
	SDL_GetWindowWMInfo(window, &window_info);

	HWND native_window = window_info.info.win.window;

	SDL_ShowWindow(window);

	ImGui::CreateContext();
	auto& imGuiIO = ImGui::GetIO();

	Renderer renderer{ native_window, WINDOW_WIDTH, WINDOW_HEIGHT };
	ImGui_ImplSDL2_InitForD3D(window);
	ImGui_ImplDX11_Init(renderer.mDevice, renderer.mDContext);

	Quad2D myQuad2 = Quad2D(renderer, INSTANCE_COUNT_DEF, false);
	myQuad2.SetInput(renderer);

	SingleQuad myQuad(renderer);
	Transform quadTransform{ .position = {0.0f, 0.0f, 0.0f}, .rotation = {0.0f, 0.0f, 0.0f}, .scale = {1.0f, 1.0f, 1.0f} };

	float aspect = WINDOW_WIDTH / WINDOW_HEIGHT;
	float windowScale = 0.08f;
	dxm::Vector3 cameraPosition = { 0.0f, 0.0f, -10.0f };
	dxm::Matrix projection = dxm::Matrix::CreateOrthographic(WINDOW_WIDTH * windowScale, WINDOW_HEIGHT * windowScale, 0.01, 1000.0f);
	//dxm::Matrix view = dxm::Matrix::CreateLookAt(cameraPosition, cameraPosition * dxm::Vector3{ 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f });

	dxm::Vector3 sunPosition{ 20.0f, 8.0f, 0.0f };
	float quadMass = 1350.0f;
	dxm::Vector3 clearColor{ 0.02f, 0.02f, 0.02f };
	renderer.SetClearColor(clearColor);
	myQuad.SetMatrix(renderer, quadTransform.GetMatrix() * dxm::Matrix::CreateTranslation(cameraPosition) * projection);
	auto frameCount = 0;
	constexpr auto frameLoop = 360;
	bool running = true;
	bool vsync = true;
	float cameraSpeed = 0.3f;
	float quadSpeed = 0.8f;
	bool mouseCaptured = false;
	Sint32 mousePrevX = 0;
	Sint32 mousePrevY = 0;
	float scroolSpeed = 0.003;
	bool randomMass = false;
	bool simpleMath = false;
	bool simulationPaused = false;

	PrepareComputeShader(renderer, myQuad2);

	while (running) {
		frameCount++;
		SDL_Event event{};
		while (SDL_PollEvent(&event) != 0) {
			ImGui_ImplSDL2_ProcessEvent(&event);

			if (!ImGui::GetIO().WantCaptureMouse) {
				switch (event.type)
				{
				case SDL_KEYDOWN: {
					handleKey(event.key.keysym.sym, SDL_KEYDOWN);
					break;
				}
				case SDL_KEYUP: {
					handleKey(event.key.keysym.sym, SDL_KEYUP);
					break;
				}
				case SDL_MOUSEWHEEL: {
					windowScale += (float)event.wheel.y * (scroolSpeed * (windowScale * 80.0f));
					if (windowScale <= 0.0f) {
						windowScale = 0.001;
					}
					break;
				}
				case SDL_MOUSEMOTION: {
					if (mouseCaptured) {
						float motionScale = 0.001;

						float xDelta = mousePrevX - event.motion.x;
						float yDelta = mousePrevY - event.motion.y;

						cameraPosition.x -= xDelta * windowScale;
						cameraPosition.y += yDelta * windowScale;
						mousePrevX = event.motion.x;
						mousePrevY = event.motion.y;
					}
					break;
				}
				case SDL_MOUSEBUTTONDOWN: {
					mouseCaptured = true;
					SDL_GetMouseState(&mousePrevX, &mousePrevY);
					break;
				}
				case SDL_MOUSEBUTTONUP: {
					mouseCaptured = false;
					break;
				}
				case SDL_QUIT: {
					running = false;
					break;
				}
				default:
					break;
				}
			}

		}
		if (keyStates[SDLK_a] == SDL_KEYDOWN) {
			cameraPosition.x += cameraSpeed;
		}
		if (keyStates[SDLK_d] == SDL_KEYDOWN) {
			cameraPosition.x -= cameraSpeed;
		}
		if (keyStates[SDLK_w] == SDL_KEYDOWN) {
			cameraPosition.y -= cameraSpeed;
		}
		if (keyStates[SDLK_s] == SDL_KEYDOWN) {
			cameraPosition.y += cameraSpeed;
		}
		if (keyStates[SDLK_z] == SDL_KEYDOWN) {
			cameraPosition.z -= cameraSpeed;
		}
		if (keyStates[SDLK_x] == SDL_KEYDOWN) {
			cameraPosition.z += cameraSpeed;
		}
		if (keyStates[SDLK_UP] == SDL_KEYDOWN) {
			sunPosition.y += quadSpeed;
		}
		if (keyStates[SDLK_DOWN] == SDL_KEYDOWN) {
			sunPosition.y -= quadSpeed;
		}
		if (keyStates[SDLK_LEFT] == SDL_KEYDOWN) {
			sunPosition.x -= quadSpeed;
		}
		if (keyStates[SDLK_RIGHT] == SDL_KEYDOWN) {
			sunPosition.x += quadSpeed;
		}
		if (keyStates[SDLK_f] == SDL_KEYDOWN) {
			//cameraPosition = sunPosition;
			std::cout << "Camera Position: " << cameraPosition.x << " " << cameraPosition.y << "\n";
			std::cout << "Sun  Position: " << sunPosition.x << " " << sunPosition.y << "\n";
			cameraPosition = sunPosition;
			cameraPosition.z = -10.0f;
		}
		const float defaultDelay = 0.25f;
		static float timeDelay = defaultDelay;
		timeDelay -= ImGui::GetIO().DeltaTime;
		if (keyStates[SDLK_p] == SDL_KEYDOWN) {
			if (timeDelay <= 0.0f) {
				simulationPaused = !simulationPaused;
				timeDelay = defaultDelay;
			}
		}

		static bool staticSun = true;
		{
			static int particleCount = INSTANCE_COUNT_DEF;
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Controls");
			ImGui::SliderFloat("Massa", &quadMass, 0.01, 10000.0f);
			ImGui::SliderFloat("Camera Speed", &cameraSpeed, 0.001f, 5.0f);
			ImGui::SliderFloat("Sun Speed", &quadSpeed, 0.001f, 5.0f);
			ImGui::SliderFloat("Zoom Level", &windowScale, 0.001f, 0.50f);
			ImGui::SliderFloat("Zoom Speed", &scroolSpeed, 0.0005f, 0.1f);
			ImGui::Checkbox("Lock the sun in place", &staticSun);
			ImGui::Checkbox("Random particle mass", &randomMass);
			if (ImGui::Button("Reset Positions") || keyStates[SDLK_SPACE] == SDL_KEYDOWN) {
				FreeComputeShader();
				myQuad2 = Quad2D(renderer, (uint32_t)particleCount, randomMass);
				cameraPosition = dxm::Vector3(0.0f, 0.0f, -10.0f);
				float xLimit = 80.0f;
				float yLimit = 40.0f;
				if (sunPosition.x > xLimit || sunPosition.x < -xLimit || sunPosition.y > yLimit || sunPosition.y < -yLimit) {
					sunPosition = dxm::Vector3{ 20.0f, 8.0f, 0.0f };
				}
				PrepareComputeShader(renderer, myQuad2);
			}

			if (ImGui::InputInt("Set Particle Count", &particleCount)) {
				if (particleCount > 0) {
					FreeComputeShader();
					myQuad2 = Quad2D(renderer, uint32_t(particleCount), randomMass);
					PrepareComputeShader(renderer, myQuad2);
				}
			}
			if (ImGui::ColorPicker3("Background Color", (float*)&clearColor)) {
				renderer.SetClearColor(clearColor);
			}
			ImGui::Checkbox("V-Sync", &vsync);

			ImGui::Text("Controls\n"
				"W A S D = Move the camera\n"
				"Arrow Keys = Move the 'sun'\n"
				"Spacebar = Reset particles position\n"
				"F = Center the camera at the sun\n"
				"Mouse Wheel = Change zoom level\n"
				"Left Click + Drag = Move the camera\n"
				"P = Play/Pause the simulation\n");
			ImGui::End();
		}

		myQuad.SetMatrix(renderer, (quadTransform.GetMatrix() * dxm::Matrix::CreateTranslation(cameraPosition) * projection).Transpose());
		//myQuad.SetMatrix(renderer, dxm::Matrix::Identity * dxm::Matrix::CreateScale(1.0f, 1.0f, 1.0f));
		projection = dxm::Matrix::CreateOrthographic(WINDOW_WIDTH * windowScale, WINDOW_HEIGHT * windowScale, 0.01, 1000.0f);
		//projection.Transpose();
		AlignedInt data{
				.instanceCount = (float)myQuad2.instanceData.size(),
				.staticSun = staticSun,
				.sunMass = quadMass,
				.sunPosition = sunPosition,
		};
		myQuad2.instanceData[0].position[0] = sunPosition.x;
		myQuad2.instanceData[0].position[1] = sunPosition.y;
		myQuad2.instanceData[0].position[2] = sunPosition.z;
		if (!simulationPaused) {
			UpdateShaderInput(renderer, myQuad2);
			DispatchComputeShader(renderer, myQuad2, data);
			ComputeShaderEndFrame(renderer, myQuad2);
		}
		/// Old calculation on the CPU; now its done on a Compute Shader
		//for (auto i = 0; i < myQuad2.instanceData.size(); i++) {
		//	auto& q = myQuad2.instanceData[i];

		//	if (i == 0) {
		//		if (staticSun) {
		//			q.position[0] = sunPosition.x;
		//			q.position[1] = sunPosition.y;
		//			q.position[2] = sunPosition.z;
		//		}
		//		else {
		//			sunPosition.x = q.position[0];
		//			sunPosition.y = q.position[1];
		//			sunPosition.z = q.position[2];
		//		}
		//		q.mass = 10000000.0f * quadMass;
		//		q.color[0] = 1.0f;
		//		q.color[1] = 1.0f;
		//		q.color[2] = 1.0f;
		//	}
		//	auto qpv = dxm::Vector3(myQuad2.instanceData[i].position[0], myQuad2.instanceData[i].position[1], myQuad2.instanceData[i].position[2]);
		//	const auto G = 6.6742E-11;


		//	for (auto y = 0; y < myQuad2.instanceData.size(); y += 1) {

		//		if (y == i) {
		//			continue;
		//		}

		//		auto& q2 = myQuad2.instanceData[y];
		//		auto q2pv = dxm::Vector3(q2.position[0], q2.position[1], q2.position[2]);
		//		auto distance = dxm::Vector3::DistanceSquared(qpv, q2pv);
		//		auto Force = G * (double(q.mass * q2.mass) / distance);
		//		//q.speed = Force / q.mass;
		//		auto d = q2pv - qpv;
		//		auto direction = d * (Force / q.mass);
		//		auto dir1 = dxm::Vector3(q.direction[0], q.direction[1], q.direction[2]);
		//		dir1 += direction;
		//		q.direction[0] = dir1.x;
		//		q.direction[1] = dir1.y;

		//	}

		//	auto direction = dxm::Vector3(q.direction[0], q.direction[1], q.direction[2]);
		//	//direction.Normalize();
		//	q.speed = direction.Length();
		//	if (q.speed > 1.0) {
		//		q.speed = 1.0f;
		//	}
		//	if (i != 0 || !staticSun) {
		//		qpv += direction;
		//		q.position[0] = qpv.x;
		//		q.position[1] = qpv.y;
		//	}

		//	if (i == 0 && staticSun) {
		//		q.mass = 1000000.0f * quadMass;
		//		q.position[0] = sunPosition.x;
		//		q.position[1] = sunPosition.y;
		//		q.position[2] = sunPosition.z;
		//		q.color[0] = 1.0f;
		//		q.color[1] = 1.0f;
		//		q.color[2] = 1.0f;
		//	}

		//	q.matrix = dxm::Matrix::Identity;
		//	if (i == 0) {
		//		float sf = quadMass * 0.005f;
		//		q.matrix *= dxm::Matrix::CreateScale(sf, sf, sf);
		//	}
		//	else {
		//		float scale = q.mass * 0.000003f;
		//		q.matrix *= dxm::Matrix::CreateScale(scale, scale, 1.0f);
		//	}
		//	q.matrix *= dxm::Matrix::CreateTranslation(qpv);
		//	q.matrix *= dxm::Matrix::CreateTranslation(cameraPosition);
		//	q.matrix *= projection;

		//	q.matrix = q.matrix.Transpose();
			//q.speed = Utils::RandNormalized();
		//}
		
		auto i = 0;
		for (auto& q : myQuad2.instanceData) {
			auto qpv = dxm::Vector3(q.position[0], q.position[1], q.position[2]);
			q.matrix = dxm::Matrix::Identity;
			if (i == 0) {
				float sf = quadMass * 0.003f;
				q.matrix *= dxm::Matrix::CreateScale(sf, sf, sf);
				if (staticSun) {
					myQuad2.instanceData[0].position[0] = sunPosition.x;
					myQuad2.instanceData[0].position[1] = sunPosition.y;
					myQuad2.instanceData[0].position[2] = sunPosition.z;
				}
			}
			else {
				float scale;
				if (q.mass > BASE_MASS * 20.0f) {
					scale = q.mass * 0.0000006f;
				}
				else {
					scale = q.mass * 0.000002f;
				}
				q.matrix *= dxm::Matrix::CreateScale(scale, scale, 1.0f);
			}
			q.matrix *= dxm::Matrix::CreateTranslation(qpv);
			q.matrix *= dxm::Matrix::CreateTranslation(cameraPosition);
			q.matrix *= projection;

			q.matrix = q.matrix.Transpose();

			i++;
		}
		if (frameCount > frameLoop) {
			frameCount = 0;
		}
		myQuad2.SetInput(renderer);
		renderer.PrepareRender();

		///Draw stuff...
		{
			//renderer.mDContext->VSSetConstantBuffers(0, 1, &instanceBuffer);
			//myQuad.Draw(renderer);
			myQuad2.Draw(renderer);
		}

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		renderer.Present(vsync);
	}

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

