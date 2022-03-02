#include "common/glm_util.h"

/// glm functions /////////////////////////////////////////////////////////////

/// \TODO: Condense both rotatePoint functions and the vec2/3/4 functions
/// \TODO: Condense rotatePoint and rotatePointXY into a single function

/**
 * Function to convert (glm::vec4) screen coordinates to glScissor friendy coordinates
 *
 * @param coords x, y coordinates to convert (z & h not applicable)
 * @param scrWidth screen width in pixels
 * @param scrHeight screen height in pixels
 * @return glScissor friendly as glm::vec4
 */
glm::vec4 convertV4ScrToGLScissor(const glm::vec4& coords, float scrWidth, float scrHeight)
{
    glm::vec4 tmp = coords;
    tmp.x += scrWidth / 2.0f;
    tmp.y += scrHeight / 2.0f;
    return tmp;
}

/**
 * Function to convert (glm::vec2) screen coordinates to normalized device coordinates
 *
 * @param coords x, y coordinates to convert
 * @param scrWidth screen width in pixels
 * @param scrHeight screen height in pixels
 * @return normalized device coordinates as glm::vec2
 */
glm::vec2 convertV2ScrToNDC(const glm::vec2& coords, float scrWidth, float scrHeight)
{
    glm::vec2 tmp2 = coords;

    // make 0 the 'center'
    tmp2.x = coords.x / (scrWidth / 2.0f) - 1;
    tmp2.y = -(coords.y / (scrHeight / 2.0f) - 1); // inverting Y

    return tmp2;
}

/**
 * Function to convert (glm::vec4) screen coordinates to normalized device coordinates
 *
 * @param coords x, y coordinates to convert (z & h not applicable)
 * @param scrWidth screen width in pixels
 * @param scrHeight screen height in pixels
 * @return normalized device coordinates as glm::vec4
 */
glm::vec4 convertV4ScrToNDC(const glm::vec4& coords, float scrWidth, float scrHeight)
{
    // store all passed floats
    glm::vec4 tmp4 = coords;

    // make 0 the 'center'
    tmp4.x = coords.x / (scrWidth / 2.0f) - 1;
    tmp4.y = -(coords.y / (scrHeight / 2.0f) - 1);

    return tmp4;
}

/**
 * Function to convert (glm::vec2) screen coordinates to 'world' friendy coordinates
 *
 * @param coords x, y coordinates to convert
 * @param scrWidth screen width in pixels
 * @param scrHeight screen height in pixels
 * @return world coordinates as glm::vec2
 */
glm::vec2 convertV2ScrToWorld(const glm::vec2& coords, float scrWidth, float scrHeight)
{
    glm::vec2 tmp2 = coords;
    tmp2.y = scrHeight - tmp2.y; // invert y
    tmp2 -= glm::vec2(scrWidth / 2.0f, scrHeight / 2.0f); // make 0 the 'center'
    return tmp2;
}

/**
 * Function to convert (glm::vec4) screen coordinates to normalized device coordinates
 *
 * @param coords x, y coordinates to convert (z & h not applicable)
 * @param scrWidth screen width in pixels
 * @param scrHeight screen height in pixels
 * @return world coordinates as glm::vec4
 */
glm::vec4 convertV4ScrToWorld(const glm::vec4& coords, float scrWidth, float scrHeight)
{
    glm::vec4 tmp4 = coords;
    tmp4.y = (scrHeight - tmp4.y) - (scrHeight / 2.0f); // invert y
    tmp4.x = scrWidth / 2.0f; // make 0 the 'center'
    return tmp4;
}

/**
 * Return slope values ("rise over run") as separate x and y values (glm::vec2)
 *
 * @param pos position to calculate against
 * @param angle angle of rotation
 * @return separate slope values (x,y)
 */
glm::vec2 getSlope(glm::vec4 pos, float angle)
{
    glm::vec2 halfDimensions(pos.z / 2.0f, pos.w / 2.0f);
    glm::vec2 positionOffset(pos.x, pos.y); // offset of x,y

    // get points centered at origin
    glm::vec2 tl(-halfDimensions.x, halfDimensions.y); // top left
    glm::vec2 bl(-halfDimensions.x, -halfDimensions.y); // bottom left
    glm::vec2 tr(halfDimensions.x, halfDimensions.y); // top right
    glm::vec2 br(halfDimensions.x, -halfDimensions.y); // bottom right

    // rotate the points
    tl = rotatePoint(tl, angle) + halfDimensions;
    bl = rotatePoint(bl, angle) + halfDimensions;
    tr = rotatePoint(tr, angle) + halfDimensions;
    br = rotatePoint(br, angle) + halfDimensions;

    return glm::vec2((tl.y - bl.y), (br.x - bl.x));
}

/// \TODO: Should this be pos.x for each??
/**
 * Function to '2d rotate' an x,y pair based on the angle, in radians
 *
 * @param pos x, y coordinates to convert
 * @param angle angle in radians
 * @return rotated point as glm::vec2
 */
glm::vec2 rotatePoint(glm::vec2 pos, float angle)
{
    pos.x = pos.x * cos(angle) - pos.y * sin(angle); // was "pos.x * ..."
    pos.y = pos.y * sin(angle) + pos.y * cos(angle); // was ALSO "pos.x * ..."
    return pos;
}

/**
 * Function to '2d rotate' an x,y,z pair (ignoring z) based on the angle, in radians
 *
 * @param pos x, y coordinates to convert
 * @param angle angle in radians
 * @return rotated point as glm::vec3
 */
glm::vec3 rotatePoint(glm::vec3 pos, float angle)
{
    pos.x = pos.x * cos(angle) - pos.y * sin(angle);
    pos.y = pos.y * sin(angle) + pos.y * cos(angle);
    pos.z = pos.z;
    return pos;
}

glm::vec3 rotatePointXY(glm::vec3 pos, float angle)
{
    pos.x = pos.x * cos(angle) - pos.y * sin(angle);
    pos.y = pos.y * sin(angle) + pos.y * cos(angle);
    pos.z = pos.z;
    return pos;
}
