function [J, grad] = costFunctionReg(theta, X, y, lambda)
%COSTFUNCTIONREG Compute cost and gradient for logistic regression with regularization
%   J = COSTFUNCTIONREG(theta, X, y, lambda) computes the cost of using
%   theta as the parameter for regularized logistic regression and the
%   gradient of the cost w.r.t. to the parameters. 

% Initialize some useful values
m = length(y); % 训练集的数目

% You need to return the following variables correctly 
J = 0;
grad = zeros(size(theta));

% ====================== YOUR CODE HERE ======================
% Instructions: Compute the cost of a particular choice of theta.
%               You should set J to the cost.
%               Compute the partial derivatives and set grad to the partial
%               derivatives of the cost w.r.t. each parameter in theta

% 下面开始计算J
h = sigmoid(X * theta)
J = (1 / m) * sum(-y .* log(h) - (1 - y) .* log(1 - h)) + (lambda / (2 * m)) * sum(power(theta, 2) - theta(1)^2)

% 然后开始计算梯度
grad = (1 / m) * (X' * (h - y)) + (lambda / m) * theta
grad(1) = (1 / m) * X(:, 1)' *(h - y)
% 或者也可以是下面的格式
%grad(1) = grad(1) - lambda / m * theta(1) 



% =============================================================

end
