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

	Quad2D myQuad2 = Quad2D(renderer, INSTANCE_COUNT_DEF);
	myQuad2.SetInput(renderer);

	SingleQuad myQuad(renderer);
	Transform quadTransform{ .position = {0.0f, 0.0f, 0.0f}, .rotation = {0.0f, 0.0f, 0.0f}, .scale = {1.0f, 1.0f, 1.0f} };

	float aspect = WINDOW_WIDTH / WINDOW_HEIGHT;
	float windowScale = 0.01f;
	dxm::Vector3 cameraPosition = { 0.0f, 0.0f, -10.0f };
	dxm::Matrix projection = dxm::Matrix::CreateOrthographic(WINDOW_WIDTH * windowScale, WINDOW_HEIGHT * windowScale, 0.01, 1000.0f);
	//dxm::Matrix view = dxm::Matrix::CreateLookAt(cameraPosition, cameraPosition * dxm::Vector3{ 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f });

	dxm::Vector3 quadMovment{ 0.0f, 0.0f, 0.0f };
	float quadMass = 1.0f;
	dxm::Vector3 clearColor{ 0.2f, 0.2f, 0.2f };
	myQuad.SetMatrix(renderer, quadTransform.GetMatrix() * dxm::Matrix::CreateTranslation(cameraPosition) * projection);
	auto frameCount = 0;
	constexpr auto frameLoop = 360;
	bool running = true;
	bool vsync = false;
	float cameraSpeed = 0.15f;
	float quadSpeed = 0.2f;
	while (running) {
		frameCount++;
		SDL_Event event{};
		while (SDL_PollEvent(&event) != 0) {
			ImGui_ImplSDL2_ProcessEvent(&event);
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

			case SDL_QUIT: {
				running = false;
				break;
			}
			default:
				break;
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
			quadMovment.y += quadSpeed;
		}
		if (keyStates[SDLK_DOWN] == SDL_KEYDOWN) {
			quadMovment.y -= quadSpeed;
		}
		if (keyStates[SDLK_LEFT] == SDL_KEYDOWN) {
			quadMovment.x -= quadSpeed;
		}
		if (keyStates[SDLK_RIGHT] == SDL_KEYDOWN) {
			quadMovment.x += quadSpeed;
		}

		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Controls");
			ImGui::SliderFloat("Massa", &quadMass, 0.01, 1000.0f);
			ImGui::SliderFloat("Camera Speed", &cameraSpeed, 0.001f, 5.0f);
			ImGui::SliderFloat("Sun Speed", &quadSpeed, 0.001f, 5.0f);
			ImGui::SliderFloat("Zoom Level", &windowScale, 0.01f, 0.10f);
			if (ImGui::Button("Reset Positions") || keyStates[SDLK_SPACE] == SDL_KEYDOWN) {
				for (auto i = 0; i < myQuad2.instanceData.size(); i++) {
					auto& q = myQuad2.instanceData[i];
					float factor = 8.0f;
					auto qpv = dxm::Vector3{ Utils::RandomRange(-factor, factor) , Utils::RandomRange(-factor, factor), .0f };;
					q.position[0] = qpv.x;
					q.position[1] = qpv.y;
				}
			}
			static int particleCount = INSTANCE_COUNT_DEF;
			if (ImGui::InputInt("Set Particle Count", &particleCount)) {
				if (particleCount > 0) {
					myQuad2 = Quad2D(renderer, uint32_t(particleCount));
				}
			}
			if (ImGui::ColorPicker3("Background Color", (float*)&clearColor)) {
				renderer.SetClearColor(clearColor);
			}
			ImGui::Checkbox("V-Sync", &vsync);

			ImGui::Text("Controls\nW A S D = Move the camera\nArrow Keys = Move the 'sun'\nSpacebar = Reset particles position\n");
			ImGui::End();
		}

		myQuad.SetMatrix(renderer, (quadTransform.GetMatrix() * dxm::Matrix::CreateTranslation(cameraPosition) * projection).Transpose());
		//myQuad.SetMatrix(renderer, dxm::Matrix::Identity * dxm::Matrix::CreateScale(1.0f, 1.0f, 1.0f));
		projection = dxm::Matrix::CreateOrthographic(WINDOW_WIDTH * windowScale, WINDOW_HEIGHT * windowScale, 0.01, 1000.0f);
		for (auto i = 0; i < myQuad2.instanceData.size(); i++) {
			auto& q = myQuad2.instanceData[i];

			if (i == 0) {
				q.mass = 1000000.0f * quadMass;
				q.position[0] = quadMovment.x;
				q.position[1] = quadMovment.y;
				q.position[2] = quadMovment.z;
				q.color[0] = 1.0f;
				q.color[1] = 1.0f;
				q.color[2] = 1.0f;
			}
			auto qpv = dxm::Vector3(myQuad2.instanceData[i].position[0], myQuad2.instanceData[i].position[1], myQuad2.instanceData[i].position[2]);
			const auto G = 6.6742E-11;

			for (auto y = i + 1; y < myQuad2.instanceData.size(); y += 1) {
				auto& q2 = myQuad2.instanceData[y];
				auto q2pv = dxm::Vector3(q2.position[0], q2.position[1], q2.position[2]);
				auto distance = dxm::Vector3::DistanceSquared(qpv, q2pv);
				auto Force = G * (double(q.mass * q2.mass) / distance);
				//q.speed = Force / q.mass;
				auto d = q2pv - qpv;
				auto direction = d * (Force / q.mass);
				auto dir1 = dxm::Vector3(q.direction[0], q.direction[1], q.direction[2]);
				dir1 += direction;
				q.direction[0] = dir1.x;
				q.direction[1] = dir1.y;

				{
					auto d2 = qpv - q2pv;
					auto direction2 = d2 * (Force / q2.mass);
					auto dir2 = dxm::Vector3(q2.direction[0], q2.direction[1], q2.direction[2]);
					dir2 += direction2;
					q2.direction[0] = dir2.x;
					q2.direction[1] = dir2.y;
					q2.speed = dir2.Length();
				}
			}

			auto direction = dxm::Vector3(q.direction[0], q.direction[1], q.direction[2]);
			//direction.Normalize();
			q.speed = direction.Length();
			if (q.speed > 1.0) {
				q.speed = 1.0f;
			}
			qpv += direction;
			q.position[0] = qpv.x;
			q.position[1] = qpv.y;

			if (frameCount == frameLoop && false) {
				float factor = 5.0f;
				qpv = dxm::Vector3{ Utils::RandomRange(-factor, factor) , Utils::RandomRange(-factor, factor), .0f };;
				q.position[0] = qpv.x;
				q.position[1] = qpv.y;
			}

			q.matrix = dxm::Matrix::Identity;
			if (i == 0) {
				q.matrix *= dxm::Matrix::CreateScale(0.6f, 0.6f, 0.6f);
			}
			else {
				q.matrix *= dxm::Matrix::CreateScale(0.22f, 0.22f, 1.0f);
			}
			q.matrix *= dxm::Matrix::CreateTranslation(qpv);
			q.matrix *= dxm::Matrix::CreateTranslation(cameraPosition);
			q.matrix *= projection;

			q.matrix = q.matrix.Transpose();
			//q.speed = Utils::RandNormalized();
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

