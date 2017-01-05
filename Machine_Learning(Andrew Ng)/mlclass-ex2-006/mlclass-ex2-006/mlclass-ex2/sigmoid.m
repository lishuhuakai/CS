function g = sigmoid(z)
%SIGMOID Compute sigmoid functoon
%   J = SIGMOID(z) computes the sigmoid of z.

% You need to return the following variables correctly 
g = zeros(size(z)); % 构建一个新的矩阵
% matlab一件不好的事情就是,没有什么类型的概念,鬼知道你传入的z是个什么东西
% ====================== YOUR CODE HERE ======================
% Instructions: Compute the sigmoid of each value of z (z can be a matrix,
%               vector or scalar).
% scalar表示标量
g = 1./(1 + exp(-z))
% =============================================================

end
