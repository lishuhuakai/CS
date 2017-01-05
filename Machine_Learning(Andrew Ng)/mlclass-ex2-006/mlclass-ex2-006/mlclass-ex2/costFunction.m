function [J, grad] = costFunction(theta, X, y)
%COSTFUNCTION Compute cost and gradient for logistic regression
%   J = COSTFUNCTION(theta, X, y) computes the cost of using theta as the
%   parameter for logistic regression and the gradient of the cost
%   w.r.t. to the parameters.

% Initialize some useful values
m = length(y); % 训练集的数目

% You need to return the following variables correctly 
J = 0; % 返回一个衡量系数
grad = zeros(size(theta)); % 这应该是最终的一个迭代结果,是吧.

% ====================== YOUR CODE HERE ======================
% Instructions: Compute the cost of a particular choice of theta.
%               You should set J to the cost.
%               Compute the partial derivatives and set grad to the partial
%               derivatives of the cost w.r.t. each parameter in theta
%
% Note: grad should have the same dimensions as theta
%
% 我们已经拥有的一些东西:X是训练集,然后y是label,我们还有初始的theta
% 最好能够向量化才行啊,兄弟

% theta应该是一个

h = sigmoid(X * theta)
J = (1 / m) * sum(-y.*log(h) - (1 - y).*log(1 - h))
% 接下来要计算梯度了
% 我们先计算其他的,最后来计算j=0的情况吧
grad = (1/m) * (X' *(h - y))




% =============================================================

end
