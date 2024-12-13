#include "vao.h"

static u32 vaos[NUM_VAOS];

void vao_init(void)
{
    glGenVertexArrays(NUM_VAOS, vaos);

    vao_bind(VAO_GUI);
    vbo_bind(VBO_GUI);
    ebo_bind(EBO_GUI);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(0 * sizeof(f32)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(2 * sizeof(f32)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(4 * sizeof(f32)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(8 * sizeof(f32)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    vao_bind(VAO_FONT);
    vbo_bind(VBO_FONT);
    ebo_bind(EBO_FONT);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(0 * sizeof(f32)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(2 * sizeof(f32)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(4 * sizeof(f32)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(8 * sizeof(f32)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
}

void vao_bind(VAO vao)
{
    glBindVertexArray(vaos[vao]);
}

void vao_destroy(void)
{
    glDeleteVertexArrays(NUM_VAOS, vaos);
}
