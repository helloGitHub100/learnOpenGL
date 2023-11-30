#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <learnopengl/animation.h>
#include <learnopengl/bone.h>
#include <thread>

class Animator
{
public:
	Animator(Animation* animation)
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = animation;

		m_FinalBoneMatrices.reserve(100);

		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}

	float GetTicksPerSecond() { return m_CurrentAnimation->GetTicksPerSecond(); }
	float GetDuration() { return m_CurrentAnimation->GetDuration(); }
	int GetBoneCount() { return m_CurrentAnimation->GetBoneIDMap().size(); }

	void getBoneMatricesForAllFrames(std::vector<vector<glm::mat4>> &boneMatricesAllFrames)
	{
		if (m_CurrentAnimation)
		{
			std::vector<std::thread> threads;
			int core_count = std::thread::hardware_concurrency();
			boneMatricesAllFrames.clear();
			int numFrames = int(m_CurrentAnimation->GetDuration() + 0.0001);
			boneMatricesAllFrames.resize(numFrames);
			cout << "Calculating bone matrices for all frames: " << numFrames << endl;
			int numFramesPerThread = numFrames / core_count;
			if (numFramesPerThread == 0)
				numFramesPerThread = numFrames;
			vector<int> startFrameVec(core_count);
			vector<int> endFrameVec(core_count);
			for (size_t i = 0, remainingFrames = numFrames; i < core_count; i++)
			{
				remainingFrames = remainingFrames - numFramesPerThread;
				int currentFrames = numFramesPerThread;
				if (i + 1 == core_count) 
					currentFrames += remainingFrames;
				int startFrame = i * numFramesPerThread;
				threads.emplace_back([=, &boneMatricesAllFrames](){
					for (size_t frameIndex = startFrame; frameIndex < startFrame + currentFrames; frameIndex++)
					{
						CalculateBoneTransformByFrameIndex(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f), frameIndex, boneMatricesAllFrames[frameIndex]);
					}
					//cout << "Frames: " << startFrame << " to " << startFrame + currentFrames << " done" << endl;
					});
			}
			for (auto& thread : threads)
				thread.join();
		}
	}

	void UpdateAnimation(float dt)
	{
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
	}

	void PlayAnimation(Animation* pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
			if (index >= m_FinalBoneMatrices.size())
				m_FinalBoneMatrices.push_back(globalTransformation * offset);
			else
				m_FinalBoneMatrices[index] = globalTransformation * offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	void CalculateBoneTransformByFrameIndex(const AssimpNodeData* node, glm::mat4 parentTransform, int frameIndex, std::vector<glm::mat4> &finalBoneMatrices)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{
			nodeTransform = Bone->getLocalTransformByFrameIndex(frameIndex);
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
			if (index >= finalBoneMatrices.size())
			{
				finalBoneMatrices.resize(index + 1);
			}
			finalBoneMatrices[index] = globalTransformation * offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransformByFrameIndex(&node->children[i], globalTransformation, frameIndex, finalBoneMatrices);
	}

	std::vector<glm::mat4> GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}

private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	Animation* m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;

};
