#version 440

layout(local_size_x = 32, local_size_y = 32) in;
layout(r8, binding = 0) uniform image2D frame;
layout(r8, binding = 1) uniform image2D bufferFrame;

uniform ivec2 simSize;

ivec2 xyLooped(int x, int y) {
	int loopedX = x;
	int loopedY = y;

	if (x < 0) loopedX = simSize.x - x;
	else if (x > simSize.x - 1) loopedX = x - simSize.x;

	if(y < 0) loopedY = simSize.y - y;
	else if (y > simSize.y - 1) loopedY = y - simSize.y;

	return ivec2(loopedX, loopedY);
}

int getCell(int x, int y) {
	return int(imageLoad(frame, xyLooped(x, y)).r);
}

void setCell(int x, int y, int value) {
	imageStore(bufferFrame, ivec2(x, y), vec4(value));
}

void main() {
	ivec2 pixelCoords = ivec2(gl_WorkGroupID.x * gl_WorkGroupSize.x + gl_LocalInvocationID.x, gl_WorkGroupID.y * gl_WorkGroupSize.y + gl_LocalInvocationID.y);

	int x = pixelCoords.x;
	int y = pixelCoords.y;

	int sum = 0;
	sum += getCell(x - 1, y - 1);
	sum += getCell(x, y - 1);
	sum += getCell(x + 1, y - 1);
	sum += getCell(x - 1, y);
	sum += getCell(x + 1, y);
	sum += getCell(x - 1, y + 1);
	sum += getCell(x, y + 1);
	sum += getCell(x + 1, y + 1);

	if (sum < 2) {
		setCell(x, y, 0);
	} else if (sum == 2 && getCell(x, y) == 1) {
		setCell(x, y, 1);
	} else if (sum == 3) {
		setCell(x, y, 1);
	} else if (sum > 3) {
		setCell(x, y, 0);
	}
}
