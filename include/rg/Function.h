//
// Created by miljan on 12/4/20.
//

#ifndef PROJECT_BASE_FUNCTION_H
#define PROJECT_BASE_FUNCTION_H
class Function {
public:
    Function() {};

    void setting_up_light(Shader &lightShader, glm::mat4 model) {
        glm::vec3 light_positions[] = {
                glm::vec3(0.0f, 1.9f, -3.5f),
                glm::vec3(0.5f, 1.9f, -3.0f),
                glm::vec3(0.0f, 1.9f, -2.5f),
                glm::vec3(-0.5f, 1.9f, -3.0f),

                glm::vec3(0.0f, 3.9f, -3.5f),
                glm::vec3(0.5f, 3.9f, -3.0f),
                glm::vec3(0.0f, 3.9f, -2.5f),
                glm::vec3(-0.5f, 3.9f, -3.0f),

                glm::vec3(0.0f, 5.9f, -3.5f),
                glm::vec3(0.5f, 5.9f, -3.0f),
                glm::vec3(0.0f, 5.9f, -2.5f),
                glm::vec3(-0.5f, 5.9f, -3.0f)
        };
        for (int i = 0; i < 12; ++i) {
            model = glm::mat4(1.0f);
            light_positions[i].x += 4.0f;
            model = glm::translate(model, light_positions[i]);
            model = glm::scale(model, glm::vec3(0.1f));
            lightShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        for (int i = 0; i < 12; ++i) {
            model = glm::mat4(1.0f);
            light_positions[i].x -= 8.0f;
            light_positions[i].z += 6.0f;
            model = glm::translate(model, light_positions[i]);
            model = glm::scale(model, glm::vec3(0.1f));
            lightShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    void setting_up_tiles_in_wall(Shader &shader, glm::mat4 model) {
        float x = 6.5f, y = 6.0f, z = -5.5f;
        for (int i = 0; i < 9; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, y, z));
            model = glm::scale(model, glm::vec3(1.5f, 0.2f, 1.5f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            x -= 1.75f;
        }

        x = 6.5f, y = 6.0f, z = 5.5f;
        for (int i = 0; i < 9; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, y, z));
            model = glm::scale(model, glm::vec3(1.5f, 0.2f, 1.5f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            x -= 1.75f;
        }
    }

    void setting_up_tiles_in_pillar(Shader &shader, glm::mat4 model) {
        // 0, -3
        // -4, 3
        float x = 4.0f, y, z = -3.0f;

        for (int j = 0; j < 2; ++j) {

            y = 0.0f;
            for (int i = 0; i < 4; ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                model = glm::scale(model, glm::vec3(1.5f, 0.1f, 1.5f));
                shader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                y += 2.0f;
            }
            x = -4.0f;
            z = 3.0f;
        }
    }

    void setting_up_floor(Shader &shader, glm::mat4 model, unsigned int floor) {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        for (int i = 0; i < 2; ++i) {
            if (i == 1) {
                glBindTexture(GL_TEXTURE_2D, floor);
            }
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, y, z));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            y = 6.0f;
        }
    }

    void setting_up_pillar(Shader& shader, glm::mat4 model, unsigned int wall) {
        float x = 4.0f;
        float z = -3.0f;
        float y;

        glBindTexture(GL_TEXTURE_2D, wall);

        for (int j = 0; j < 2; ++j) {
            y = 0.5f;
            for (int i = 0; i < 6; ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                shader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                y += 1.0f;
            }
            x = -4.0f;
            z = 3.0f;
        }
    }

    void setting_up_wall(Shader &shader, glm::mat4 model, unsigned int tile, unsigned int wall, float height) {
        float x, y = height, z;

        for (int j = 0; j < 6; ++j) {
            glBindTexture(GL_TEXTURE_2D, wall);
            if (j == 1 || j == 4) {
                glBindTexture(GL_TEXTURE_2D, tile);
            }

            x = 6.5f, z = -5.5f;
            for (int i = 0; i < 14; ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                shader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                x -= 1.0f;
            }

            for (int i = 0; i <= 10; ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                shader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                z += 1.0f;
            }

            for (int i = 0; i <= 14; ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                shader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                x += 1.0f;
            }

            y += 1.0f;
        }
    }
};

#endif //PROJECT_BASE_FUNCTION_H