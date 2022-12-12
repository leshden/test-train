#include "framework/engine.h"
#include "framework/utils.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/spline.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp>

using namespace std;
using namespace glm;

Mesh createRail(const vector<glm::vec3>& points_spine, float offset)
{
	vector<Vertex>       vertices;
	vector<unsigned int> indices;

	const int SIZE_V = points_spine.size();

	for (int i{ 0 }; i < SIZE_V; ++i) {
		vec3 res = points_spine[i] - points_spine[(i + 1) >= SIZE_V ? 0 : (i + 1)];
		float ang_rad = atan2(res.z, res.x);
		float angle = ang_rad * 180.f / glm::pi<float>();

		glm::vec3 euler = glm::vec3(glm::radians(-90.f), glm::radians(-angle), glm::radians(0.f));

		glm::quat qu = glm::quat(euler);

		glm::mat4 t = glm::mat4(1.0f);
		t = glm::translate(t, points_spine[i]);
		t = t * glm::mat4_cast(qu);
		t = glm::scale(t, glm::vec3(1, 1, 1));

		vec4 left_down = t * glm::vec4(0.f, offset, 0, 1);
		//vec4 right_down = t * glm::vec4(0.5f, -0.5f, 0, 1);
		//vec4 right_top = t * glm::vec4(0.5f, 0.5f, 0, 1);
		vec4 left_top = t * glm::vec4(0.f, offset + 0.15f, 0, 1);

		vertices.push_back({ glm::vec3(left_down.x, -left_down.z, 0), glm::vec3(0,0,1), glm::vec2(1,1) });
		vertices.push_back({ glm::vec3(left_top.x, -left_top.z, 0), glm::vec3(0,0,1), glm::vec2(1,1) });
	}

	for (int i{ 0 }; i < (2 * (SIZE_V - 1)); i += 2) {
		indices.push_back(i + 1);
		indices.push_back(i + 3);
		indices.push_back(i + 2);

		indices.push_back(i + 1);
		indices.push_back(i + 2);
		indices.push_back(i);
	}

	int last_index = 2 * SIZE_V;

	indices.push_back(last_index - 1);
	indices.push_back(1);
	indices.push_back(0);

	indices.push_back(last_index - 1);
	indices.push_back(0);
	indices.push_back(last_index - 2);

	std::reverse(vertices.begin(), vertices.end());

	return Mesh(vertices, indices);
}


Mesh createTie()
{
	vector<Vertex>       vertices;
	vector<unsigned int> indices;

	vertices.push_back({ glm::vec3(-0.25f, -0.5f, 0), glm::vec3(0,0,1), glm::vec2(0,0) });
	vertices.push_back({ glm::vec3(0.f, -0.5f, 0), glm::vec3(0,0,1), glm::vec2(1,0) });
	vertices.push_back({ glm::vec3(0.f, 0.5f, 0), glm::vec3(0,0,1), glm::vec2(1,1) });
	vertices.push_back({ glm::vec3(-0.25f, 0.5f, 0), glm::vec3(0,0,1), glm::vec2(0,1) });

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	return Mesh(vertices, indices);
}



class Railcar {
public:
	Railcar(Object* obj):m_object(obj){}

	void update(float dt, vector<glm::vec3>* points) {

		if (a >= 1.f) {
			++point;
			a = 0.f;
			if (point >= points->size()) {
				point = 0;
			}
		}

		int p1 = point;
		int p2 = (point == (points->size() - 1)) ? 0 : point + 1;

		glm::vec3 new_pos = glm::lerp((*points)[p1], (*points)[p2], a);

		vec3 old_pos = m_object->getPosition();
		m_object->setPosition(new_pos);

		vec3 res = new_pos - old_pos;

		float distance = glm::distance((*points)[p1], (*points)[p2]);
		float ang_rad = atan2(res.z, res.x);
		float angle = ang_rad * 180.f / glm::pi<float>();

		m_object->setRotation(0.f, -angle, 0.f);

		a += (dt * speed) / distance;
	}

	void setOffset(int point, float a) {
		this->point = point;
		this->a = a;
	}

private:
	int point = 0;
	float speed = 3.f;
	float a = 0.0f;
	glm::vec3 dir = vec3(1.f, 0.f, 0.f);
	Object* m_object = nullptr;

};


class Train {
public:
	Train(){}
	void addRailcar(Railcar* railcar) {
		railcars.push_back(railcar);
	}

	~Train() {
		for (Railcar* railcar : railcars) {
			delete railcar;
		}
	}

	void update(float dt, vector<glm::vec3>* points) {
		for (Railcar* railcar : railcars) {
			railcar->update(dt, points);
		}
	}

private:
	vector<Railcar*> railcars;

};

void makeTrain(Train& train) {

}

/*
* Coordinate system:
* x - right
* y - up
* z - backward
*/

int main()
{
	// initialization
	Engine* engine = Engine::get();
	engine->init(1600, 900, "UNIGINE Test Task");

	// set up camera
	Camera& cam = engine->getCamera();
	cam.Position = vec3(0.0f, 12.0f, 17.0f);
	cam.Yaw = -90.0f;
	cam.Pitch = -45.0f;
	cam.UpdateCameraVectors();

	// create shared meshes
	Mesh plane_mesh = createPlane();
	Mesh sphere_mesh = createSphere();

	// create background objects
	Object* plane = engine->createObject(&plane_mesh);
	plane->setColor(0.2f, 0.37f, 0.2f); // green
	plane->setPosition(0, -0.5f, 0);
	plane->setRotation(-90.0f, 0.0f, 0.0f);
	plane->setScale(20.0f);

	// path
	const float path[] = {
		 0.0f, -0.375f,  7.0f, // 1
		-6.0f, -0.375f,  5.0f, // 2
		-8.0f, -0.375f,  1.0f, // 3
		-4.0f, -0.375f, -6.0f, // 4
		 0.0f, -0.375f, -7.0f, // 5
		 1.0f, -0.375f, -4.0f, // 6
		 4.0f, -0.375f, -3.0f, // 7
		 8.0f, -0.375f,  7.0f  // 8
	};

	const float axis_x[] = {
	 -10.0f, -0.375f,  0.0f, // 1
	 10.0f, -0.375f,  0.0f, // 2
	};

	const float axis_z[] = {
	 0.0f, -0.375f,  -10.0f, // 1
	 0.0f, -0.375f,  10.0f, // 2
	};

	vector<Object*> points;
	vector<glm::vec3> points_spine;
	vector<glm::vec3> points_test;

	const float OFFSET = 0.1f;

	// --- init points curve ----
	// start path
	for (float i{ 0.f }; i < 1.f; i += OFFSET) {
		points_spine.push_back(glm::catmullRom(
			glm::vec3(path[21], path[22], path[23]),
			glm::vec3(path[0], path[1], path[2]),
			glm::vec3(path[3], path[4], path[5]),
			glm::vec3(path[6], path[7], path[8]), i));
	}

	// middle path
	for (int i{ 0 }; i <= 12; i += 3) {
		for (float j{ 0.f }; j < 1.f; j += OFFSET) {
			points_spine.push_back(glm::catmullRom(
				glm::vec3(path[i + 0], path[i + 1], path[i + 2]),
				glm::vec3(path[i + 3], path[i + 4], path[i + 5]),
				glm::vec3(path[i + 6], path[i + 7], path[i + 8]),
				glm::vec3(path[i + 9], path[i + 10], path[i + 11]), j));
		}
	}

	for (float i{ 0.f }; i < 1.f; i += OFFSET) {
		points_spine.push_back(glm::catmullRom(
			glm::vec3(path[15], path[16], path[17]),
			glm::vec3(path[18], path[19], path[20]),
			glm::vec3(path[21], path[22], path[23]),
			glm::vec3(path[0], path[1], path[2]), i));
	}

	// end path
	for (float i{ 0.f }; i < 1.f; i += OFFSET) {
		points_spine.push_back(glm::catmullRom(
			glm::vec3(path[18], path[19], path[20]),
			glm::vec3(path[21], path[22], path[23]),
			glm::vec3(path[0], path[1], path[2]),
			glm::vec3(path[3], path[4], path[5]), i));
	}
	//--- end init points curve ----

	for (int i = 0; i < 8; i++)
	{
		Object* sphere = engine->createObject(&sphere_mesh);
		sphere->setColor(1, 0, 0);
		sphere->setPosition(path[i * 3], path[i * 3 + 1], path[i * 3 + 2]);
		points_test.push_back(glm::vec3(path[i * 3], path[i * 3 + 1], path[i * 3 + 2]));
		sphere->setScale(0.25f);
		points.push_back(sphere);

	}
	LineDrawer path_drawer(path, points.size(), true);

	// --- code here ---
	LineDrawer path_drawer_spine(points_spine, true);
	path_drawer_spine.setColor(0.0f, 1.0f, 0.f);

	LineDrawer line_x(axis_x, 2, false);
	LineDrawer line_z(axis_z, 2, false);

	Train train;
	Mesh cube_mesh = createCube();
	for (int i{ 0 }; i < 4; ++i) {
		Object* cube = engine->createObject(&cube_mesh);
		cube->setColor(0, 0, 1);
		cube->setPosition(points_spine[0]);
		Railcar* railcar = new Railcar(cube);
		railcar->setOffset(i * 2, 0.0f);
		train.addRailcar(railcar);
	}
	
	//--- Rails ----
	Mesh rail_mesh_1 = createRail(points_spine, -0.4f);

	Object* rail1 = engine->createObject(&rail_mesh_1);
	rail1->setColor(1.f, 0.0f, 0.2f); 
	rail1->setPosition(vec3(0.0f, -0.36f, 0.0f));
	rail1->setRotation(-90.0f, 0.f, 0.0f);
	rail1->setScale(1.f);


	Mesh rail_mesh_2 = createRail(points_spine, 0.2f);

	Object* rail2 = engine->createObject(&rail_mesh_2);
	rail2->setColor(1.f, 0.0f, 0.2f);
	rail2->setPosition(vec3(0.0f, -0.36f, 0.0f));
	rail2->setRotation(-90.0f, 0.f, 0.0f);
	rail2->setScale(1.f);


	//--- Rail tie -----
	vector<glm::vec3> points_tie;
	const float OFFSET_TIE = 0.01f;

	// start path
	for (float i{ 0.f }; i < 1.f; i += OFFSET_TIE) {

		glm::vec3 tmp = glm::catmullRom(
			glm::vec3(path[21], path[22], path[23]),
			glm::vec3(path[0], path[1], path[2]),
			glm::vec3(path[3], path[4], path[5]),
			glm::vec3(path[6], path[7], path[8]), i);

		if (points_tie.size() > 0) {
			vec3 distance = points_tie[points_tie.size() - 1] -  tmp;
			float L = length(distance);

			if (L < 0.5f) {
				continue;
			}
		}

		points_tie.push_back(tmp);
	}

	// middle path
	for (int i{ 0 }; i <= 12; i += 3) {
		for (float j{ 0.f }; j < 1.f; j += OFFSET_TIE) {

			glm::vec3 tmp = glm::catmullRom(
				glm::vec3(path[i + 0], path[i + 1], path[i + 2]),
				glm::vec3(path[i + 3], path[i + 4], path[i + 5]),
				glm::vec3(path[i + 6], path[i + 7], path[i + 8]),
				glm::vec3(path[i + 9], path[i + 10], path[i + 11]), j);

			if (points_tie.size() > 1) {
				vec3 distance = points_tie[points_tie.size() - 1] - tmp;

				float L = length(distance);

				if (L < 0.5f) {
					continue;
				}
			}

			points_tie.push_back(tmp);
		}
	}

	for (float i{ 0.f }; i < 1.f; i += OFFSET_TIE) {

		glm::vec3 tmp = glm::catmullRom(
			glm::vec3(path[15], path[16], path[17]),
			glm::vec3(path[18], path[19], path[20]),
			glm::vec3(path[21], path[22], path[23]),
			glm::vec3(path[0], path[1], path[2]), i);

		if (points_tie.size() > 1) {
			vec3 distance = points_tie[points_tie.size() - 1] - tmp;

			float L = length(distance);

			if (L < 0.5f) {
				continue;
			}
		}

		points_tie.push_back(tmp);
	}

	// end path
	for (float i{ 0.f }; i < 1.f; i += OFFSET_TIE) {

		glm::vec3 tmp = glm::catmullRom(
			glm::vec3(path[18], path[19], path[20]),
			glm::vec3(path[21], path[22], path[23]),
			glm::vec3(path[0], path[1], path[2]),
			glm::vec3(path[3], path[4], path[5]), i);

		if (points_tie.size() > 1) {
			vec3 distance = tmp - points_tie.back();

			float L = length(distance);

			if (L < 0.5f) {
				continue;
			}
		}

		points_tie.push_back(tmp);
	}

	Mesh tie_mesh = createTie();

	for (int i{ 0 }; i < points_tie.size(); ++i) {
		Object* tie = engine->createObject(&tie_mesh);
		tie->setColor(0.f, 1.0f, 1.0f); // green
		tie->setPosition(points_tie[i]);

		vec3 res = points_tie[(i == (points_tie.size() - 1) ? 0 : i + 1)] - points_tie[i];

		float ang_rad = atan2(res.z, res.x);
		float angle = ang_rad * 180.f / glm::pi<float>();


		tie->setRotation(-90.0f, -angle, 0.0f);
		tie->setScale(1.f);
	}
	// ---- end Rails ----

	// -- up train 
	vector<glm::vec3> points_spine_train = points_spine;
	for (int i{ 0 }; i < points_spine_train.size(); ++i) {
		points_spine_train[i].y = 0.37f;
	}
	
	// main loop
	while (!engine->isDone())
	{
		engine->update();
		train.update(engine->getDeltaTime(), &points_spine_train);

		engine->render();
		path_drawer.draw();
		path_drawer_spine.draw();

		line_z.draw();
		line_x.draw();
		
		engine->swap();
	}

	engine->shutdown();
	return 0;
}
