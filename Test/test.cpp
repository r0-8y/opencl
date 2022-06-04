#include <CL/cl.hpp>
#include <iostream>
#include <fstream>

int main()
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	auto platform = platforms.front();
	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

	auto device = devices.front();
	auto vendor = device.getInfo<CL_DEVICE_VENDOR>();
	auto version = device.getInfo<CL_DEVICE_VERSION>();
	auto name = device.getInfo<CL_DEVICE_NAME>();

	std::cout << name << std::endl;
	std::cout << vendor << std::endl;
	std::cout << version << std::endl;

	std::ifstream helloWorldFile("test.cl");
	std::string src(std::istreambuf_iterator<char>(helloWorldFile), (std::istreambuf_iterator<char>()));

	cl::Program::Sources sources(1, std::make_pair(src.c_str(), src.length() + 1));
	cl::Context context(device);

	cl::Program program(context, sources);

	auto error = program.build(devices);

	char buf[16];
	cl::Buffer memBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(buf));
	cl::Kernel kernel(program, "HelloWorld", &error);
	kernel.setArg(0, memBuf);

	cl::CommandQueue queue(context, device);
	queue.enqueueTask(kernel);
	queue.enqueueReadBuffer(memBuf, CL_TRUE, 0, sizeof(buf), buf);

	std::cout << buf;

	return 0;
}