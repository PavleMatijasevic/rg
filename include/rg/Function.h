//
// Created by miljan on 12/4/20.
//

#ifndef PROJECT_BASE_FUNCTION_H
#define PROJECT_BASE_FUNCTION_H

#include <learnopengl/model.h>

class Function {
public:
    Function() {};

    void load_stairs(Model &stairs_model, glm::mat4 &model, Shader &shader) {
        model = glm::translate(model, glm::vec3(-4.1f, -0.5f, 2.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.2f, 1.0f));
        shader.setMat4("model", model);
        stairs_model.Draw(shader);
    }

    void load_sofa(Model &sofa_model, glm::mat4 &model, Shader &shader) {
        model = glm::translate(model, glm::vec3(2.0f, 0.01f, -3.7f));
        model = glm::scale(model, glm::vec3(0.0015f, 0.002f, 0.002f));
        shader.setMat4("model", model);
        sofa_model.Draw(shader);
    }

    void setting_up_light(Shader &lightShader, glm::mat4 &model) {
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
        int n = 12;

        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            light_positions[i].x -= 4.0f;
            model = glm::translate(model, light_positions[i]);
            model = glm::scale(model, glm::vec3(0.1f));
            lightShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            light_positions[i].x += 8.0f;
            light_positions[i].z += 6.0f;
            model = glm::translate(model, light_positions[i]);
            model = glm::scale(model, glm::vec3(0.1f));
            lightShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    void setting_up_tiles_in_wall(Shader &shader, glm::mat4 &model) {
        float tiles_in_wall_x_positions[] = {
                6.5f, 4.75f, 3.0f, 1.25f, -0.5f, -2.25f, -4.0f, -5.75f, -7.5f
        };
        float y = 6.0f, z = -5.5f;
        int n = 9;

        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(tiles_in_wall_x_positions[i], y, z));
            model = glm::scale(model, glm::vec3(1.5f, 0.1f, 1.5f));
            shader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        z *= -1;
        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(tiles_in_wall_x_positions[i], y, z));
            model = glm::scale(model, glm::vec3(1.5f, 0.1f, 1.5f));
            shader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    void setting_up_tiles_in_pillar(Shader &shader, glm::mat4 &model) {
        // 0, -3
        // -4, 3
        float x = -4.0f, y, z = -3.0f;
        int m = 2, n = 4;

        for (int j = 0; j < m; ++j) {

            y = 0.0f;
            for (int i = 0; i < n; ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                model = glm::scale(model, glm::vec3(1.5f, 0.1f, 1.5f));
                shader.setMat4("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);
                y += 2.0f;
            }
            x = 4.0f;
            z = 3.0f;
        }
    }

    void setting_up_floor(Shader &shader, glm::mat4 &model, unsigned int floor) {
        glBindTexture(GL_TEXTURE_2D, floor);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.8f, 6.0f, 0.0f));
        model = glm::scale(model,glm::vec3(0.6f, 1.0f, 1.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.2f, 6.0f, -1.25f));
        model = glm::scale(model, glm::vec3(0.4f, 1.0f, 0.75f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    void setting_up_pillar(Shader& shader, glm::mat4 &model, unsigned int wall) {
        int n = 6;
        float y = 0.5f;

        glBindTexture(GL_TEXTURE_2D, wall);
        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-4.0f, y, -3.0f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            y += 1.0f;
        }

        y = 0.5f;
        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(4.0f, y, 3.0f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            y += 1.0f;
        }
    }

    void setting_up_wall(Shader &shader, glm::mat4 &model, unsigned int tile, unsigned int wall, float height) {
        float x, y = height, z;
        int m = 6, n = 14, k = 11;

        for (int j = 0; j < m; ++j) {
            glBindTexture(GL_TEXTURE_2D, wall);
            if (j == 1 || j == 4) {
                glBindTexture(GL_TEXTURE_2D, tile);
            }

            x = 6.5f, z = -5.5f;
            for (int i = 0; i < n; ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                shader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                x -= 1.0f;
            }

            for (int i = 0; i < k; ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                shader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                z += 1.0f;
            }

            for (int i = 0; i <= n; ++i) {
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



