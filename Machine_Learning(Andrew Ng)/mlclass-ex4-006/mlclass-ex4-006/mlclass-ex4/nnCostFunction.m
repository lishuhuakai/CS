function [J grad] = nnCostFunction(nn_params, ...
                                   input_layer_size, ...
                                   hidden_layer_size, ...
                                   num_labels, ...
                                   X, y, lambda)
%好吧,终于轮到我开始写代价函数了.还要开始计算梯度了.
%NNCOSTFUNCTION Implements the neural network cost function for a two layer
%neural network which performs classification
%   [J grad] = NNCOSTFUNCTON(nn_params, hidden_layer_size, num_labels, ...
%   X, y, lambda) computes the cost and gradient of the neural network. The
%   parameters for the neural network are "unrolled" into the vector
%   nn_params and need to be converted back into the weight matrices. 
% 
%   The returned parameter grad should be a "unrolled" vector of the
%   partial derivatives of the neural network.
%

% Reshape nn_params back into the parameters Theta1 and Theta2, the weight matrices
% for our 2 layer neural network
Theta1 = reshape(nn_params(1:hidden_layer_size * (input_layer_size + 1)), ...
                 hidden_layer_size, (input_layer_size + 1));

Theta2 = reshape(nn_params((1 + (hidden_layer_size * (input_layer_size + 1))):end), ...
                 num_labels, (hidden_layer_size + 1));

% Setup some useful variables
m = size(X, 1);
         
% You need to return the following variables correctly 
J = 0;
Theta1_grad = zeros(size(Theta1));
Theta2_grad = zeros(size(Theta2));

% ====================== YOUR CODE HERE ======================
% Instructions: You should complete the code by working through the
%               following parts.
%
% Part 1: Feedforward the neural network and return the cost in the
%         variable J. After implementing Part 1, you can verify that your
%         cost function computation is correct by verifying the cost
%         computed in ex4.m
%
% Part 2: Implement the backpropagation algorithm to compute the gradients
%         Theta1_grad and Theta2_grad. You should return the partial derivatives of
%         the cost function with respect to Theta1 and Theta2 in Theta1_grad and
%         Theta2_grad, respectively. After implementing Part 2, you can check
%         that your implementation is correct by running checkNNGradients
%
%         Note: The vector y passed into the function is a vector of labels
%               containing values from 1..K. You need to map this vector into a 
%               binary vector of 1's and 0's to be used with the neural network
%               cost function.
%
%         Hint: We recommend implementing backpropagation using a for-loop
%               over the training examples if you are implementing it for the 
%               first time.
%
% Part 3: Implement regularization with the cost function and gradients.
%
%         Hint: You can implement this around the code for
%               backpropagation. That is, you can compute the gradients for
%               the regularization separately and then add them to Theta1_grad
%               and Theta2_grad from Part 2.
%

% 好吧,貌似这个计算过程非常复杂.
% 首先要进行前向传播,计算预测的结果
a1 = [ones(m, 1) X]; % m * (n + 1)
% 第一层的输入是n + 1个数据
z2 = a1 * Theta1'; % 得到第二层的数据, 在这里z2是一个5000 * 25类型的矩阵
a2 = [ones(m, 1) sigmoid(z2)]; % a2是一个5000 *　26类型的矩阵,第一列全是bias unit
z3 = a2 * Theta2';
a3 = sigmoid(z3); % 这个玩意是预测的结果

% 现在要开始计算J了
for i = 1:m
    h = a3(i, :)';
    J = J + sum(log(1 - h)) + log(h(y(i, :))) - log(1 - h(y(i, :)));
end

J = 1/m * J;
 
squareTheta1 = Theta1.^2;
squareTheta1(:, 1) = 0;
squareTheta2 = Theta2.^2;
squareTheta2(:, 1) = 0;
 
J = J + lambda/(2 * m) * (sum(sum(squareTheta1)) + sum(sum(squareTheta2)));
 

% 下面要开始反向神经网络的计算而来

delta3 = zeros(num_labels, 1); % delta3是一个10 * 1的矩阵
% 反向传播,输出层的误差
delta2 = zeros(size(Theta1)); % delta2是一个25 * 401型的矩阵
% 反向传播,隐藏层的误差

 

delta3 = a3; % 现在delta3是一个5000 * 10类型的矩阵
% 现在要减去1啦
for i = 1:m
    delta3(i, y(i)) = delta3(i, y(i)) - 1;
end
% delta3现在计算好了,现在我们要计算delta2

delta2 = Theta2' * delta3 .* [1; sigmoidGradient(z2)];


   


    



















% -------------------------------------------------------------

% =========================================================================

% Unroll gradients
grad = [Theta1_grad(:) ; Theta2_grad(:)];


end
