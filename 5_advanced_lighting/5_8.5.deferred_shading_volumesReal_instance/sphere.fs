#version 330 core
out vec4 FragColor;

void main()
{
    //FragColor = vec4(1.0, 1.0, 1.0, 1.0);//白色：用于测试深度测试的效果
    //FragColor = vec4(0.0, 0.0, 0.0, 1.0);//黑色：用于测试深度测试的效果
    //如果main函数为空，输出为透明无色，在其后渲染的其他物体仍会深度测试失败
}